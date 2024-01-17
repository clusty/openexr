import os
import argparse
import glob
import re
import json


def get_headers(hdir):
    return list(glob.glob(os.path.join(hdir, "Imf*Compressor.h"), recursive=False))


def parse_header(hfile, codecs):
    print(f" -- parsing {hfile}...")
    # read the header
    with open(hfile, "r") as fh:
        f = fh.read()
    # parse the declarations
    l = re.findall(r"/\*\s*(REGISTER[\s\w\d\n\-,\.:]+)\*/", f, re.MULTILINE | re.DOTALL)
    ints = ("id", "numScanlines")
    for m in l:
        # print(repr(m))
        codec = {"enum": re.search(r"REGISTER\s+(\w+)", m).group(1)}
        for ma in re.findall(r"(\w+):\s*(.+)\n", m, re.MULTILINE):
            codec[ma[0]] = ma[1] if ma[0] not in ints else int(ma[1])
        # print(codec)
        codecs.append(codec)


def sanity_check_codecs(codecs):
    # make sure codec ids are unique
    codec_ids = []
    for codec in codecs:
        if codec["id"] not in codec_ids:
            codec_ids.append(codec["id"])
        else:
            print(json.dumps(codecs, indent=3))
            raise BaseException("Duplicate codec id detected !")


def enum_definition_string(codecs):
    out = ""
    for cdc in codecs:
        out += "    %(enum)s = %(id)s, // %(desc)s\n" % cdc
    out += "    NUM_COMPRESSION_METHODS // number of different compression methods"
    return out


def codec_declaration_string(codecs):
    out = ""
    for cdc in codecs:
        out += (
            "    {Compression::%(enum)s,\n"
            "     CompressionDesc (\n"
            '        "%(name)s",\n'
            '        "%(desc)s",\n'
            "        %(numScanlines)s,\n"
            "        %(lossy)s)},\n"
        ) % cdc
    return out[:-2]  # remove trailing comma


def name_to_id_string(codecs):
    out = ""
    for cdc in codecs:
        out += '    {"%(name)s", Compression::%(enum)s},\n' % cdc
        # special case because 'no' and 'none' are accepted in cmd line tools.
        if cdc["name"] == "none":
            out += '    {"no", Compression::%(enum)s},\n' % cdc
    return out[:-2]  # remove trailing comma


def generate(args):
    codecs = [
        {
            "enum": "NO_COMPRESSION",
            "id": 0,
            "name": "none",
            "desc": "no compression",
            "numScanlines": 1,
            "lossy": "false",
        }
    ]

    with open(args.input, "r") as fh:
        out_file = fh.read()

    hdir = os.path.dirname(args.IMFCOMPRESSION_H)
    for hfile in get_headers(hdir):
        parse_header(hfile, codecs)

    sanity_check_codecs(codecs)
    # sort codecs by id
    codecs.sort(key=lambda x: x["id"])
    # print(json.dumps(codecs, indent=3))

    out_file = out_file.replace(
        "// CMAKE_ENUM_DEFINITION", enum_definition_string(codecs)
    )
    out_file = out_file.replace(
        "// CMAKE_CODEC_DECLARATIONS", codec_declaration_string(codecs)
    )
    out_file = out_file.replace(
        "// CMAKE_CODEC_NAME_TO_ID", name_to_id_string(codecs)
    )
    # print(out_file)

    with open(args.IMFCOMPRESSION_H, "w") as fh:
        fh.write(out_file)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Parses all Imf*Compressor.h files to register all compression methods."
    )
    parser.add_argument("IMFCOMPRESSION_H", help="The generated header file.")
    parser.add_argument(
        "-i",
        "--input",
        help="The input header file to be updated.",
    )
    args = parser.parse_args()
    generate(args)
