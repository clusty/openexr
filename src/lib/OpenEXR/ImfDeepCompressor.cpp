//
// Created by vladal on 27/10/23.
//

#include <cstring>
#include <zstd.h>
#include "ImfDeepCompressor.h"

namespace
{

int
ZSTD_compress_impl (const char* inPtr, int inSize, Imf::DeepCompressor::raw_ptr& outPtr)
{
    auto const cBuffSize = ZSTD_compressBound(inSize);
    outPtr = Imf::DeepCompressor::raw_ptr((char*)malloc (cBuffSize), &free);
    auto const cSize = ZSTD_compress(outPtr.get(), cBuffSize, inPtr, inSize, 15);
    return cSize;
}

int
ZSTD_uncompress_impl (const char* inPtr, int inSize, Imf::DeepCompressor::raw_ptr& outPtr)
{
    auto rSize = ZSTD_getFrameContentSize(inPtr, inSize);
    outPtr = Imf::DeepCompressor::raw_ptr((char*)malloc (rSize), &free);
    auto const dSize = ZSTD_decompress(outPtr.get(), rSize, inPtr, inSize);
    return dSize;
}


}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER
DeepCompressor::DeepCompressor ( const Header& hdr, size_t maxScanLines): Compressor(hdr), _maxScanLines (maxScanLines),_outBuffer (nullptr, &free)
{}

int
DeepCompressor::numScanLines () const
{
    return 256; // max ? // Needs to be in sync with ImfCompressor::numLinesInBuffer
}
int
DeepCompressor::compress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto ret = ZSTD_compress_impl(inPtr, inSize, _outBuffer);
    outPtr = _outBuffer.get();
    return ret;
}
int
DeepCompressor::uncompress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto ret = ZSTD_uncompress_impl(inPtr, inSize, _outBuffer);
    outPtr = _outBuffer.get();
    return ret;
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT