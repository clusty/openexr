//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

/* REGISTER RLE_COMPRESSION
id: 1
name: rle
desc: run-length encoding
numScanlines: 1
lossy: false
*/

#ifndef INCLUDED_IMF_RLE_COMPRESSOR_H
#define INCLUDED_IMF_RLE_COMPRESSOR_H

//-----------------------------------------------------------------------------
//
//	class RleCompressor -- performs run-length encoding
//
//-----------------------------------------------------------------------------

#include "ImfNamespace.h"

#include "ImfCompressor.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class RleCompressor : public Compressor
{
public:
    RleCompressor (const Header& hdr, size_t maxScanLineSize);
    virtual ~RleCompressor ();

    RleCompressor (const RleCompressor& other)            = delete;
    RleCompressor& operator= (const RleCompressor& other) = delete;
    RleCompressor (RleCompressor&& other)                 = delete;
    RleCompressor& operator= (RleCompressor&& other)      = delete;

    virtual int numScanLines () const;

    virtual int
    compress (const char* inPtr, int inSize, int minY, const char*& outPtr);

    virtual int
    uncompress (const char* inPtr, int inSize, int minY, const char*& outPtr);

private:
    int   _maxScanLineSize;
    char* _tmpBuffer;
    char* _outBuffer;
};

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
