//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

#ifndef INCLUDED_IMF_COMPRESSION_H
#define INCLUDED_IMF_COMPRESSION_H

//-----------------------------------------------------------------------------
//
//	enum Compression
//
//-----------------------------------------------------------------------------
#include "ImfExport.h"
#include "ImfNamespace.h"
#include <string>
#include <cstring>
#include <map>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

enum IMF_EXPORT_ENUM Compression
{
    NO_COMPRESSION = 0, // no compression

    RLE_COMPRESSION = 1, // run-length encoding.

    ZIPS_COMPRESSION = 2, // zlib compression, one scan line at a time.

    ZIP_COMPRESSION = 3, // zlib compression, in blocks of 16 scan lines.

    PIZ_COMPRESSION = 4, // piz-based wavelet compression, in blocks of 32 scan
                         // lines.

    PXR24_COMPRESSION = 5, // lossy 24-bit float compression, in blocks of 16
                           // scan lines.

    B44_COMPRESSION = 6, // lossy 4-by-4 pixel block compression, fixed
                         // compression rate.

    B44A_COMPRESSION = 7, // lossy 4-by-4 pixel block compression, flat fields
                          // are compressed more.

    DWAA_COMPRESSION = 8, // lossy DCT based compression, in blocks of 32
                          // scanlines. More efficient for partial buffer
                          // access.

    DWAB_COMPRESSION = 9, // lossy DCT based compression, in blocks of 256
                          // scanlines. More efficient space wise and faster to
                          // decode full frames than DWAA_COMPRESSION.

    ZSTD_COMPRESSION = 10, // zstandard compression, in blocks of 32 scan
                           // lines.

    NUM_COMPRESSION_METHODS // number of different compression methods
};

struct CompressionDesc
{
    std::string name;
    std::string desc;
    int         numScanlines;
    bool        lossy;
    bool        deep;

    CompressionDesc (
        std::string _name,
        std::string _desc,
        int         _scanlines,
        bool        _lossy,
        bool        _deep)
    {
        name         = _name;
        desc         = _desc;
        numScanlines = _scanlines;
        lossy        = _lossy;
        deep         = _deep;
    }
};

// clang-format off
static const std::map<Compression, CompressionDesc> IdToDesc = {
    {Compression::NO_COMPRESSION,
     CompressionDesc (
        "none",
        "no compression",
        1,
        false,
        true)},
    {Compression::RLE_COMPRESSION,
     CompressionDesc (
        "rle",
        "run-length encoding.",
        1,
        false,
        true)},
    {Compression::ZIPS_COMPRESSION,
     CompressionDesc (
        "zips",
        "zlib compression, one scan line at a time.",
        1,
        false,
        true)},
    {Compression::ZIP_COMPRESSION,
     CompressionDesc (
        "zip",
        "zlib compression, in blocks of 16 scan lines.",
        16,
        false,
        false)},
    {Compression::PIZ_COMPRESSION,
     CompressionDesc (
        "piz",
        "piz-based wavelet compression, in blocks of 32 scan lines.",
        32,
        false,
        false)},
    {Compression::PXR24_COMPRESSION,
     CompressionDesc (
        "pxr24",
        "lossy 24-bit float compression, in blocks of 16 scan lines.",
        16,
        true,
        false)},
    {Compression::B44_COMPRESSION,
     CompressionDesc (
        "b44",
        "lossy 4-by-4 pixel block compression, fixed compression rate.",
        32,
        true,
        false)},
    {Compression::B44A_COMPRESSION,
     CompressionDesc (
        "b44a",
        "lossy 4-by-4 pixel block compression, flat fields are compressed more.",
        32,
        true,
        false)},
    {Compression::DWAA_COMPRESSION,
     CompressionDesc (
        "dwaa",
        "lossy DCT based compression, in blocks of 32 scanlines. More efficient for partial buffer access.",
        32,
        true,
        false)},
    {Compression::DWAB_COMPRESSION,
     CompressionDesc (
        "dwab",
        "lossy DCT based compression, in blocks of 256 scanlines. More efficient space wise and faster to decode full frames than DWAA_COMPRESSION.",
        256,
        true,
        false)},
    {Compression::ZSTD_COMPRESSION,
     CompressionDesc (
        "zstd",
        "zstandard compression, in blocks of 32 scan lines.",
        32,
        false,
        true)}
};
// clang-format on

static const std::map<std::string, Compression> CompressionNameToId = {
    {"none", Compression::NO_COMPRESSION},
    {"no", Compression::NO_COMPRESSION},
    {"rle", Compression::RLE_COMPRESSION},
    {"zips", Compression::ZIPS_COMPRESSION},
    {"zip", Compression::ZIP_COMPRESSION},
    {"piz", Compression::PIZ_COMPRESSION},
    {"pxr24", Compression::PXR24_COMPRESSION},
    {"b44", Compression::B44_COMPRESSION},
    {"b44a", Compression::B44A_COMPRESSION},
    {"dwaa", Compression::DWAA_COMPRESSION},
    {"dwab", Compression::DWAB_COMPRESSION},
    {"zstd", Compression::ZSTD_COMPRESSION}
};

#define UNKNOWN_COMPRESSION_ID_MSG "INVALID COMPRESSION ID"

/// Returns a codec ID's short name (lowercase).
IMF_EXPORT inline void
getCompressionNameFromId (const Compression id, std::string& name)
{
    auto it = IdToDesc.find (id);
    name = it != IdToDesc.end () ? it->second.name : UNKNOWN_COMPRESSION_ID_MSG;
}

/// Returns a codec ID's short description (lowercase).
IMF_EXPORT inline void
getCompressionDescriptionFromId (const Compression id, std::string& desc)
{
    auto it = IdToDesc.find (id);
    desc    = it != IdToDesc.end () ? it->second.name + ": " + it->second.desc
                                    : UNKNOWN_COMPRESSION_ID_MSG;
}

/// Returns the codec name's ID, NUM_COMPRESSION_METHODS if not found.
IMF_EXPORT inline void
getCompressionIdFromName (const std::string& name, Compression& id)
{
    std::string lowercaseName (name);
    for (auto& ch: lowercaseName)
        ch = std::tolower (ch);

    auto it = CompressionNameToId.find (lowercaseName);
    id      = it != CompressionNameToId.end ()
                  ? it->second
                  : Compression::NUM_COMPRESSION_METHODS;
}

/// Return true if a compression id exists.
IMF_EXPORT inline bool
isValidCompressionId (const Compression id)
{
    return IdToDesc.find (id) != IdToDesc.end ();
}

/// Return a string enumerating all compression names, with a custom separator.
IMF_EXPORT inline void
getCompressionNamesString (const std::string separator, std::string& in)
{
    for (auto it = IdToDesc.begin (); it != std::prev (IdToDesc.end ()); ++it)
    {
        in += it->second.name + separator;
    }
    in += std::prev (IdToDesc.end ())->second.name;
}

/// Return the number of scan lines expected by a given compression method.
IMF_EXPORT inline int
getCompressionNumScanlines (const Compression id)
{
    auto it = IdToDesc.find (id);
    return it != IdToDesc.end () ? it->second.numScanlines : -1;
}

/// Return true is the compression method does not preserve the data's integrity.
IMF_EXPORT inline bool
isLossyCompressionId (const Compression id)
{
    auto it = IdToDesc.find (id);
    return it != IdToDesc.end () ? it->second.lossy : false;
}

/// Return true is the compression method supports deep data.
IMF_EXPORT inline bool
isDeepCompressionId (const Compression id)
{
    auto it = IdToDesc.find (id);
    return it != IdToDesc.end () ? it->second.deep : false;
}

/// Controls the default zip compression level used. Zip is used for
/// the 2 zip levels as well as some modes of the DWAA/B compression.
IMF_EXPORT void setDefaultZipCompressionLevel (int level);

/// Controls the default quality level for the DWA lossy compression
IMF_EXPORT void setDefaultDwaCompressionLevel (float level);

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
