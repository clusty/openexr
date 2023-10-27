//
// Created by vladal on 27/10/23.
//

#include <cstring>
#include "ImfZstdCompressor.h"
OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER
ZstdCompressor::ZstdCompressor ( const Header& hdr, int maxScanLineSize): Compressor(hdr), _maxScanLineSize(maxScanLineSize),_outBuffer (nullptr, &free)
{}
int
ZstdCompressor::numScanLines () const
{
    return 1; // max ? // Needs to be in sync with ImfCompressor::numLinesInBuffer
}
int
ZstdCompressor::compress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    _outBuffer = raw_ptr((char*)malloc (inSize), &free);
    memcpy(_outBuffer.get(), inPtr, inSize);
    outPtr = _outBuffer.get();
    return inSize;
}
int
ZstdCompressor::uncompress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    _outBuffer = raw_ptr((char*)malloc (inSize), &free);
    memcpy(_outBuffer.get(), inPtr, inSize);
    outPtr = _outBuffer.get();
    return inSize;
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT