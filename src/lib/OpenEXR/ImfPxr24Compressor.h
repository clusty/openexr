//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Pixar Animation Studios and Contributors of the OpenEXR Project
//

// Mandatory section to register this compression method in OpenEXR
// ----------------------------------------------------------------
// PXR24_COMPRESSION = 5
// PXR24_COMPRESSION name pxr24
// PXR24_COMPRESSION desc lossy 24-bit float compression, in blocks of 16 scan lines.
// PXR24_COMPRESSION scanlines 16
// PXR24_COMPRESSION lossy true
// PXR24_COMPRESSION deep false
// PXR24_COMPRESSION newscan Pxr24Compressor (hdr, maxScanLineSize, 16)
// PXR24_COMPRESSION newtile Pxr24Compressor (hdr, tileLineSize, numTileLines)

#ifndef INCLUDED_IMF_PXR24_COMPRESSOR_H
#define INCLUDED_IMF_PXR24_COMPRESSOR_H

//-----------------------------------------------------------------------------
//
//	class Pxr24Compressor -- Loren Carpenter's 24-bit float compressor
//
//-----------------------------------------------------------------------------

#include "ImfCompressor.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class Pxr24Compressor : public Compressor
{
public:
    Pxr24Compressor (
        const Header& hdr, size_t maxScanLineSize, size_t numScanLines);

    virtual ~Pxr24Compressor ();

    Pxr24Compressor (const Pxr24Compressor& other)            = delete;
    Pxr24Compressor& operator= (const Pxr24Compressor& other) = delete;
    Pxr24Compressor (Pxr24Compressor&& other)                 = delete;
    Pxr24Compressor& operator= (Pxr24Compressor&& other)      = delete;

    virtual int numScanLines () const;

    virtual Format format () const;

    virtual int
    compress (const char* inPtr, int inSize, int minY, const char*& outPtr);

    virtual int compressTile (
        const char*            inPtr,
        int                    inSize,
        IMATH_NAMESPACE::Box2i range,
        const char*&           outPtr);

    virtual int
    uncompress (const char* inPtr, int inSize, int minY, const char*& outPtr);

    virtual int uncompressTile (
        const char*            inPtr,
        int                    inSize,
        IMATH_NAMESPACE::Box2i range,
        const char*&           outPtr);

private:
    int compress (
        const char*            inPtr,
        int                    inSize,
        IMATH_NAMESPACE::Box2i range,
        const char*&           outPtr);

    int uncompress (
        const char*            inPtr,
        int                    inSize,
        IMATH_NAMESPACE::Box2i range,
        const char*&           outPtr);

    int                _maxScanLineSize;
    int                _numScanLines;
    unsigned char*     _tmpBuffer;
    char*              _outBuffer;
    const ChannelList& _channels;
    int                _minX;
    int                _maxX;
    int                _maxY;
};

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
