//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

#ifndef INCLUDED_IMF_COMPRESSION_H
#define INCLUDED_IMF_COMPRESSION_H

//-----------------------------------------------------------------------------
//
//  enum Compression
// 
// This file enumerates available compression methods and defines a simple API 
// to query them.
//
//-----------------------------------------------------------------------------

#include "ImfForward.h"
#include <string>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

// All available compression methods.
// NOTE: Must be extended to add a new codec.
enum IMF_EXPORT_ENUM Compression
{
    NO_COMPRESSION    = 0,
    RLE_COMPRESSION   = 1,
    ZIPS_COMPRESSION  = 2,
    ZIP_COMPRESSION   = 3,
    PIZ_COMPRESSION   = 4,
    PXR24_COMPRESSION = 5,
    B44_COMPRESSION   = 6,
    B44A_COMPRESSION  = 7,
    DWAA_COMPRESSION  = 8,
    DWAB_COMPRESSION  = 9,
    NUM_COMPRESSION_METHODS
};

/// Returns a codec ID's short name (lowercase).
IMF_EXPORT void getCompressionNameFromId (Compression id, std::string& name);

/// Returns a codec ID's short description (lowercase).
IMF_EXPORT void
getCompressionDescriptionFromId (Compression id, std::string& desc);

/// Returns the codec name's ID, NUM_COMPRESSION_METHODS if not found.
IMF_EXPORT void
getCompressionIdFromName (const std::string& name, Compression& id);

/// Return true if a compression id exists.
IMF_EXPORT bool isValidCompressionId (Compression id);

/// Return a string enumerating all compression names, with a custom separator.
IMF_EXPORT void
getCompressionNamesString (const std::string& separator, std::string& in);

/// Return the number of scan lines expected by a given compression method.
IMF_EXPORT int getCompressionNumScanlines (Compression id);

/// Return true is the compression method does not preserve the data's integrity.
IMF_EXPORT bool isLossyCompressionId (Compression id);

/// Return true is the compression method supports deep data.
IMF_EXPORT bool isDeepCompressionId (Compression id);

/// Controls the default zip compression level used. Zip is used for
/// the 2 zip levels as well as some modes of the DWAA/B compression.
IMF_EXPORT void setDefaultZipCompressionLevel (int level);

/// Controls the default quality level for the DWA lossy compression
IMF_EXPORT void setDefaultDwaCompressionLevel (float level);

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
