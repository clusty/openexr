//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

/* REGISTER ZIPS_COMPRESSION
id: 2
name: zips
desc: zlib compression, one scan line at a time
numScanlines: 1
lossy: false
*/

/* REGISTER ZIP_COMPRESSION
id: 3
name: zip
desc: zlib compression, in blocks of 16 scan lines
numScanlines: 16
lossy: false
*/


#ifndef INCLUDED_IMF_ZIP_COMPRESSOR_H
#define INCLUDED_IMF_ZIP_COMPRESSOR_H

//-----------------------------------------------------------------------------
//
//	class ZipCompressor -- performs zlib-style compression
//
//-----------------------------------------------------------------------------

#include "ImfNamespace.h"

#include "ImfCompressor.h"

#include "ImfZip.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class ZipCompressor : public Compressor
{
public:
    ZipCompressor (
        const Header& hdr, size_t maxScanLineSize, size_t numScanLines);

    virtual ~ZipCompressor ();

    virtual int numScanLines () const;

    virtual int
    compress (const char* inPtr, int inSize, int minY, const char*& outPtr);

    virtual int
    uncompress (const char* inPtr, int inSize, int minY, const char*& outPtr);

private:
    int   _maxScanLineSize;
    int   _numScanLines;
    char* _outBuffer;
    Zip   _zip;
};

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
