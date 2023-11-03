//
// Created by vladal on 27/10/23.
//

#include <cstring>
#include "ImfDeepCompressor.h"
#include <zstd.h>
#include "blosc2.h"
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

#define NTHREADS 4

int
BLOSC_compress_impl (const char* inPtr, int inSize, Imf::DeepCompressor::raw_ptr& outPtr)
{
    blosc2_init();
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    cparams.typesize = sizeof(int32_t);
    cparams.clevel = 9;
    cparams.nthreads = NTHREADS;
    dparams.nthreads = NTHREADS;

    blosc2_storage storage = {.cparams=&cparams, .dparams=&dparams};
    blosc2_schunk* schunk = blosc2_schunk_new(&storage);
    auto in = const_cast<char*>(inPtr);
    int64_t nchunks = blosc2_schunk_append_buffer(schunk, in, inSize);

    uint8_t *buffer;
    bool yours = false;
    auto size = blosc2_schunk_to_buffer(schunk, &buffer, &yours);
    outPtr = Imf::DeepCompressor::raw_ptr((char*)buffer, &free);

    blosc2_schunk_free(schunk);
    blosc2_destroy();

    return size;
}

int
BLOSC_uncompress_impl (const char* inPtr, int inSize, Imf::DeepCompressor::raw_ptr& outPtr, int maxScanLineSize)
{
    blosc2_init();
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    cparams.typesize = sizeof(int32_t);
    cparams.clevel = 9;
    cparams.nthreads = NTHREADS;
    dparams.nthreads = NTHREADS;

    blosc2_storage storage = {.cparams=&cparams, .dparams=&dparams};
    auto in = const_cast<char*>(inPtr);
    blosc2_schunk* schunk = blosc2_schunk_from_buffer(reinterpret_cast<uint8_t*>(in), inSize, false);//(schunk, in, inSize);

    auto buffSize = maxScanLineSize * 256;
    outPtr = Imf::DeepCompressor::raw_ptr((char*)malloc (buffSize), &free);
    auto size = blosc2_schunk_decompress_chunk(schunk, 0, outPtr.get(), buffSize);

    blosc2_schunk_free(schunk);
    blosc2_destroy();

    return size;
}

}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER
DeepCompressor::DeepCompressor ( const Header& hdr, size_t maxScanLines): Compressor(hdr), _maxScanLines (maxScanLines),_outBuffer (nullptr, &free), _hdr(hdr)
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
    _hdr.sanityCheck();
    auto ret = BLOSC_compress_impl(inPtr, inSize, _outBuffer);
    outPtr = _outBuffer.get();
    return ret;
}
int
DeepCompressor::uncompress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto ret = BLOSC_uncompress_impl(inPtr, inSize, _outBuffer, _maxScanLines);
    outPtr = _outBuffer.get();
    return ret;
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT