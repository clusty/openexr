import os
import argparse
import glob
import re
import json


def get_headers(hdir):
    return list(glob.glob(os.path.join(hdir, "Imf*Compressor.h"), recursive=False))


def parse_header(hfile, codecs):
    # read the header
    with open(hfile, "r") as fh:
        f = fh.read()
    # parse the declarations
    matches = re.findall(
        r"/\*\s*(REGISTER[\s\w\d\n\-,\.:]+)\*/", f, re.MULTILINE | re.DOTALL
    )
    ints = ("id", "numScanlines")
    for m in matches:
        codec = {"enum": re.search(r"REGISTER\s+(\w+)", m).group(1)}
        for ma in re.findall(r"(\w+):\s*(.+)\n", m, re.MULTILINE):
            codec[ma[0]] = ma[1] if ma[0] not in ints else int(ma[1])
        codecs.append(codec)


def sanity_check_codecs(codecs):
    # make sure codec ids are unique
    codec_ids = []
    for codec in codecs:
        if codec["id"] not in codec_ids:
            codec_ids.append(codec["id"])
        else:
            print(json.dumps(codecs, indent=3))
            raise RuntimeError("Duplicate codec id detected !")


def wrap_align_comments(line):
    if len(line) > 80:
        MAX_LEN = 81
        cut = line.rfind(" ", 0, MAX_LEN)
        indent = " " * line.find("//") + "// "
        l1 = indent + line[cut:].strip(" ")
        line = line[:cut].rstrip(" ") + "\n"
        while len(l1) > 80:
            cut = l1.rfind(" ", 0, MAX_LEN)
            line += l1[:cut] + "\n"
            l1 = indent + l1[cut:].strip(" ")
        if len(l1) > len(indent):
            line += l1
    return line


def enum_definition_string(codecs):
    out = ""
    for cdc in codecs:
        out += wrap_align_comments("    %(enum)s = %(id)s, // %(desc)s\n\n" % cdc)
    out += "    NUM_COMPRESSION_METHODS // number of different compression methods."
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
            "        %(lossy)s,\n"
            "        %(deep)s)},\n"
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
            "deep": "true",
        }
    ]

    # read template file
    with open(args.input, "r") as fh:
        out_file = fh.read()

    # parse all headers
    hdir = os.path.dirname(args.OUPPUT_FILE)
    for hfile in get_headers(hdir):
        parse_header(hfile, codecs)

    sanity_check_codecs(codecs)
    # sort codecs by id
    codecs.sort(key=lambda x: x["id"])

    # replace template tokens
    out_file = out_file.replace(
        "// CMAKE_ENUM_DEFINITION", enum_definition_string(codecs)
    )
    out_file = out_file.replace(
        "// CMAKE_CODEC_DECLARATIONS", codec_declaration_string(codecs)
    )
    out_file = out_file.replace("// CMAKE_CODEC_NAME_TO_ID", name_to_id_string(codecs))

    # save to disk
    if os.path.exists(args.OUPPUT_FILE):
        # omly save if different
        with open(args.OUPPUT_FILE, "r") as fh:
            old_file = fh.read()
        if old_file == out_file:
            return

    with open(args.OUPPUT_FILE, "w") as fh:
        fh.write(out_file)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Parses all Imf*Compressor.h files to register all compression methods."
    )
    parser.add_argument("OUPPUT_FILE", help="The generated header file.")
    parser.add_argument(
        "-i",
        "--input",
        help="The input header file to be updated.",
    )
    args = parser.parse_args()
    generate(args)
