//
// Created by vladal on 27/10/23.
//

#include <cstring>
#include <zstd.h>
#include "ImfZstdCompressor.h"

#define NTHREADS 4

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER
ZstdCompressor::ZstdCompressor ( const Header& hdr, size_t maxScanLines): Compressor(hdr), _maxScanLines (maxScanLines),_outBuffer (nullptr, &free)
{}

int
ZstdCompressor::numScanLines () const
{
    return 256; // max ? // Needs to be in sync with ImfCompressor::numLinesInBuffer
}
int
ZstdCompressor::compress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
   /* _outBuffer = raw_ptr((char*)malloc (inSize), &free);
    memcpy(_outBuffer.get(), inPtr, inSize);
    outPtr = _outBuffer.get();*/

    auto const cBuffSize = ZSTD_compressBound(inSize);
    _outBuffer = raw_ptr((char*)malloc (cBuffSize), &free);
    auto const cSize = ZSTD_compress(_outBuffer.get(), cBuffSize, inPtr, inSize, 15);
    outPtr = _outBuffer.get();
    return cSize;
}
int
ZstdCompressor::uncompress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
   /* _outBuffer = raw_ptr((char*)malloc (inSize), &free);
    memcpy(_outBuffer.get(), inPtr, inSize);
    outPtr = _outBuffer.get();*/
    auto rSize = ZSTD_getFrameContentSize(inPtr, inSize);
    _outBuffer = raw_ptr((char*)malloc (rSize), &free);
    auto const dSize = ZSTD_decompress(_outBuffer.get(), rSize, inPtr, inSize);
    outPtr = _outBuffer.get();
    return dSize;
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT