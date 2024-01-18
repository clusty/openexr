import os
import argparse
import glob
import re
import json
import logging


def setup_logger(verbose):
    levels = [logging.WARNING, logging.INFO, logging.DEBUG]
    logging.basicConfig(
        level=levels[min(verbose, 2)],
        format="[%(filename)s] %(levelname)9s: %(message)s",
    )


def get_headers(hdir):
    return list(glob.glob(os.path.join(hdir, "Imf*Compressor.h"), recursive=False))


def parse_header(hfile, codecs):
    # read the header
    with open(hfile, "r") as fh:
        f = fh.read()
    # parse the declarations
    matches = re.findall(
        r"/\*\s*(REGISTER[^*]+)\*/", f, re.MULTILINE | re.DOTALL
    )
    ints = ("id", "numScanlines")
    for m in matches:
        logging.debug('  > extracted: %r', m)
        codec = {"enum": re.search(r"REGISTER\s+(\w+)", m).group(1)}
        for ma in re.findall(r"(\w+):\s*(.+)\n", m, re.MULTILINE):
            codec[ma[0]] = ma[1] if ma[0] not in ints else int(ma[1])
        logging.debug('  > dict: %r', codec)
        codecs.append(codec)


def sanity_check_codecs(codecs):
    # make sure codec ids are unique
    codec_ids = []
    for codec in codecs:
        if codec["id"] not in codec_ids:
            codec_ids.append(codec["id"])
        else:
            print(json.dumps(codecs, indent=3))
            raise RuntimeError("Duplicate codec id detected ! See printed dict above.")


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
    logging.info("Read template file: %s", args.input)

    # parse all headers
    hdir = os.path.dirname(args.OUTPUT_FILE)
    for hfile in get_headers(hdir):
        parse_header(hfile, codecs)
        logging.info("Parsed header: %s", hfile)

    sanity_check_codecs(codecs)
    logging.info("sanity_check_codecs: PASSED")
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
    logging.info("All tokens replaced.")

    # save to disk
    if os.path.exists(args.OUTPUT_FILE):
        # omly save if different
        with open(args.OUTPUT_FILE, "r") as fh:
            old_file = fh.read()
        if old_file == out_file:
            logging.info("Same file: do not write to disk.")
            return

    with open(args.OUTPUT_FILE, "w") as fh:
        fh.write(out_file)
    logging.info("File saved: %s", args.OUTPUT_FILE)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Parses all Imf*Compressor.h files to register all compression methods."
    )
    parser.add_argument("OUTPUT_FILE", help="The generated header file.")
    parser.add_argument(
        "-i",
        "--input",
        help="The input header file to be updated.",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        type=int,
        default=0,
        help="Print extra infos. 0: silent, 1: infos, 2: debug",
    )
    args = parser.parse_args()
    setup_logger(args.verbose)
    generate(args)
