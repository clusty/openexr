//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

//-----------------------------------------------------------------------------
//
//      exrenvmap -- makes OpenEXR environment maps
//
//-----------------------------------------------------------------------------

#include <EnvmapImage.h>
#include <ImfEnvmap.h>
#include <ImfHeader.h>
#include <ImfMisc.h>
#include <OpenEXRConfig.h>

#include <blurImage.h>
#include <makeCubeMap.h>
#include <makeLatLongMap.h>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "namespaceAlias.h"
using namespace IMF;
using namespace std;

namespace
{

void
usageMessage (ostream& stream, const char* program_name, bool verbose = false)
{
    stream << "Usage: " << program_name << " [options] infile outfile" << endl;

    if (verbose)
    {
        std::string compressionNames;
        getCompressionNamesString ("/", compressionNames);

        stream
            << "\n"
               "Convert an OpenEXR latitude-longitude environment map\n"
               "into a cube-face environment map or vice versa.\n"
               "Reads an environment map image from infile, converts\n"
               "it, and stores the result in outfile.\n"
               "\n"
               "If the input file name contains a '%' character, then an\n"
               "input cube-face environment map is assembled from six\n"
               "square sub-images that represent the six faces of the cube.\n"
               "The names of the six image files are generated by replacing\n"
               "the % with +X, -X, +Y, -Y, +Z and -Z respectively.\n"
               "\n"
               "If the output file name contains a '%' character and\n"
               "the program has been instructed to produce a cube-face\n"
               "environment map, then the output image is split into six\n"
               "square sub-images that are saved in six separate output\n"
               "files.  The names of the files are generated by replacing\n"
               "the % with +X, -X, +Y, -Y, +Z and -Z respectively.\n"
               "\n"
               "Options:\n"
               "\n"
               "  -o            produces a ONE_LEVEL output file (default)\n"
               "\n"
               "  -m            produces a MIPMAP_LEVELS output file (-m has\n"
               "                no effect if the output image is split into\n"
               "                multiple files)\n"
               "\n"
               "  -c            the output file will be a cube-face environment\n"
               "                map (default)\n"
               "\n"
               "  -l            the output file will be a latitude-longitude\n"
               "                environment map\n"
               "\n"
               "  -ci           the input file is interpreted as a cube-face\n"
               "                environment map, regardless of its envmap\n"
               "                attribute\n"
               "\n"
               "  -li           the input file is interpreted as a latitude-\n"
               "                longitude environment map, regardless of its\n"
               "                envmap attribute (-li has no effect if the\n"
               "                input image is assembled from multiple files)\n"
               "\n"
               "  -w x          sets the width of the output image to x pixels\n"
               "                (default is 256).  The height of the output image\n"
               "                will be x*6 pixels for a cube-face map, or x/2\n"
               "                pixels for a latitude-longitude map.\n"
               "\n"
               "  -f r n        sets the antialiasing filter radius to r\n"
               "                (default is 1.0) and the sampling rate to\n"
               "                n by n (default is 5 by 5).  Increasing r\n"
               "                makes the output image blurrier; decreasing r\n"
               "                makes the image sharper but may cause aliasing.\n"
               "                Increasing n improves antialiasing, but\n"
               "                generating the output image takes longer.\n"
               "\n"
               "  -b            blurs the environment map image by applying a\n"
               "                180-degree-wide filter kernel such that point-\n"
               "                sampling the blurred image at a location that\n"
               "                corresponds to 3D direction N returns the color\n"
               "                that a white diffuse reflector with surface\n"
               "                normal N would have if it was illuminated using\n"
               "                the original non-blurred image.\n"
               "                Generating the blurred image can be fairly slow.\n"
               "\n"
               "  -t x y        sets the output file's tile size to x by y pixels\n"
               "                (default is 64 by 64)\n"
               "\n"
               "  -p t b        if the input image is a latitude-longitude map,\n"
               "                pad the image at the top and bottom with t*h\n"
               "                and b*h extra scan lines, where h is the height\n"
               "                of the input image.  This is useful for images\n"
               "                from 360-degree panoramic scans that cover\n"
               "                less than 180 degrees vertically.\n"
               "\n"
               "  -d            sets level size rounding to ROUND_DOWN (default)\n"
               "\n"
               "  -u            sets level size rounding to ROUND_UP\n"
               "\n"
               "  -z x          sets the data compression method to x\n"
               "                ("
            << compressionNames.c_str ()
            << ",\n"
               "                default is zip)\n"
               "\n"
               "  -v            verbose mode\n"
               "\n"
               "  -h, --help    print this message\n"
               "\n"
               "      --version print version information\n"
               "\n"
               "Report bugs via https://github.com/AcademySoftwareFoundation/openexr/issues or email security@openexr.com\n"
               "";
    }
}

Compression
getCompression (const string& str)
{
    Compression c;
    std::string lowerStr = str;
    getCompressionIdFromName (lowerStr, c);
    if (c == Compression::NUM_COMPRESSION_METHODS)
    {
        std::stringstream e;
        e << "Unknown compression method \"" << str << "\"";
        throw invalid_argument (e.str ().c_str ());
    }

    return c;
}

} // namespace

int
main (int argc, char** argv)
{
    const char*       inFile            = 0;
    const char*       outFile           = 0;
    Envmap            type              = ENVMAP_CUBE;
    Envmap            overrideInputType = NUM_ENVMAPTYPES;
    LevelMode         levelMode         = ONE_LEVEL;
    LevelRoundingMode roundingMode      = ROUND_DOWN;
    Compression       compression       = ZIP_COMPRESSION;
    int               mapWidth          = 256;
    int               tileWidth         = 64;
    int               tileHeight        = 64;
    float             padTop            = 0;
    float             padBottom         = 0;
    float             filterRadius      = 1;
    int               numSamples        = 5;
    bool              diffuseBlur       = false;
    bool              verbose           = false;

    //
    // Parse the command line.
    //

    if (argc < 2)
    {
        usageMessage (cerr, argv[0], false);
        return -1;
    }

    try
    {
        int i = 1;

        while (i < argc)
        {
            if (!strcmp (argv[i], "-o"))
            {
                //
                // generate a ONE_LEVEL image
                //

                levelMode = ONE_LEVEL;
                i += 1;
            }
            else if (!strcmp (argv[i], "-m"))
            {
                //
                // Generate a MIPMAP_LEVELS image
                //

                levelMode = MIPMAP_LEVELS;
                i += 1;
            }
            else if (!strcmp (argv[i], "-c"))
            {
                //
                // Generate a cube-face map
                //

                type = ENVMAP_CUBE;
                i += 1;
            }
            else if (!strcmp (argv[i], "-l"))
            {
                //
                // Generate a latitude-longitude map
                //

                type = ENVMAP_LATLONG;
                i += 1;
            }
            else if (!strcmp (argv[i], "-ci"))
            {
                //
                // Assume input is a cube-face map
                //

                overrideInputType = ENVMAP_CUBE;
                i += 1;
            }
            else if (!strcmp (argv[i], "-li"))
            {
                //
                // Assume input is a latitude-longitude map
                //

                overrideInputType = ENVMAP_LATLONG;
                i += 1;
            }
            else if (!strcmp (argv[i], "-w"))
            {
                //
                // Set output image width
                //

                if (i > argc - 2)
                    throw invalid_argument ("Missing width for -w argument");

                mapWidth = strtol (argv[i + 1], 0, 0);

                if (mapWidth <= 0)
                    throw invalid_argument (
                        "Output image width must be greater than zero");

                i += 2;
            }
            else if (!strcmp (argv[i], "-f"))
            {
                //
                // Set filter radius and supersampling rate
                //

                if (i > argc - 3)
                    throw invalid_argument (
                        "Missing filter radius with -f option");

                filterRadius = strtod (argv[i + 1], 0);
                numSamples   = strtol (argv[i + 2], 0, 0);

                if (filterRadius < 0)
                    throw invalid_argument (
                        "Filter radius must not be less than zero");

                if (numSamples <= 0)
                    throw invalid_argument (
                        "Sampling rate must be greater than zero");

                i += 3;
            }
            else if (!strcmp (argv[i], "-b"))
            {
                //
                // Diffuse blur
                //

                diffuseBlur = true;
                i += 1;
            }
            else if (!strcmp (argv[i], "-t"))
            {
                //
                // Set tile size
                //

                if (i > argc - 3)
                    throw invalid_argument ("missing tile size with -t option");

                tileWidth  = strtol (argv[i + 1], 0, 0);
                tileHeight = strtol (argv[i + 2], 0, 0);

                if (tileWidth <= 0 || tileHeight <= 0)
                    throw invalid_argument (
                        "Tile size must be greater than zero");

                i += 3;
            }
            else if (!strcmp (argv[i], "-p"))
            {
                //
                // Set top and bottom padding
                //

                if (i > argc - 3)
                    throw invalid_argument (
                        "missing padding value with -p option");

                padTop    = strtod (argv[i + 1], 0);
                padBottom = strtod (argv[i + 2], 0);

                if (padTop < 0 || padBottom < 0)
                    throw invalid_argument (
                        "Padding must not be less than zero");

                i += 3;
            }
            else if (!strcmp (argv[i], "-d"))
            {
                //
                // Round down
                //

                roundingMode = ROUND_DOWN;
                i += 1;
            }
            else if (!strcmp (argv[i], "-u"))
            {
                //
                // Round down
                //

                roundingMode = ROUND_UP;
                i += 1;
            }
            else if (!strcmp (argv[i], "-z"))
            {
                //
                // Set compression method
                //

                if (i > argc - 2)
                    throw invalid_argument (
                        "Missing compression method with -z option");

                compression = getCompression (argv[i + 1]);
                i += 2;
            }
            else if (!strcmp (argv[i], "-v"))
            {
                //
                // Verbose mode
                //

                verbose = true;
                i += 1;
            }
            else if (!strcmp (argv[i], "-h") || !strcmp (argv[i], "--help"))
            {
                //
                // Print help message
                //

                usageMessage (cout, "exrenvmap", true);
                return 0;
            }
            else if (!strcmp (argv[i], "--version"))
            {
                const char* libraryVersion = getLibraryVersion ();

                cout << "exrenvmap (OpenEXR) " << OPENEXR_VERSION_STRING;
                if (strcmp (libraryVersion, OPENEXR_VERSION_STRING))
                    cout << "(OpenEXR version " << libraryVersion << ")";
                cout << " https://openexr.com" << endl;
                cout << "Copyright (c) Contributors to the OpenEXR Project"
                     << endl;
                cout << "License BSD-3-Clause" << endl;
                return 0;
            }
            else
            {
                //
                // Image file name
                //

                if (inFile == 0)
                    inFile = argv[i];
                else
                    outFile = argv[i];

                i += 1;
            }
        }

        if (inFile == 0 || outFile == 0)
        {
            usageMessage (cerr, argv[0], false);
            return -1;
        }

        //
        // Load inFile, convert it, and save the result in outFile.
        //

        EnvmapImage  image;
        Header       header;
        RgbaChannels channels;

        readInputImage (
            inFile,
            padTop,
            padBottom,
            overrideInputType,
            verbose,
            image,
            header,
            channels);

        if (diffuseBlur) blurImage (image, verbose);

        if (type == ENVMAP_CUBE)
        {
            makeCubeMap (
                image,
                header,
                channels,
                outFile,
                tileWidth,
                tileHeight,
                levelMode,
                roundingMode,
                compression,
                mapWidth,
                filterRadius,
                numSamples,
                verbose);
        }
        else
        {
            makeLatLongMap (
                image,
                header,
                channels,
                outFile,
                tileWidth,
                tileHeight,
                levelMode,
                roundingMode,
                compression,
                mapWidth,
                filterRadius,
                numSamples,
                verbose);
        }
    }
    catch (const exception& e)
    {
        cerr << argv[0] << ": " << e.what () << endl;
        return 1;
    }

    return 0;
}
