//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

//-----------------------------------------------------------------------------
//
//	class ZstdCompressor
//
//-----------------------------------------------------------------------------

#include "ImfZstdCompressor.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER

ZstdCompressor::ZstdCompressor (const Header& hdr, size_t maxScanLineSize, int numScanLines)
    : Compressor (hdr, EXR_COMPRESSION_ZSTD, maxScanLineSize, numScanLines)
{
}

ZstdCompressor::~ZstdCompressor ()
{
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
