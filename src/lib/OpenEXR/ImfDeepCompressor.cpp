//
// Created by vladal on 27/10/23.
//

#include <cstring>
#include "ImfDeepCompressor.h"
#include "IlmThread.h"
//#include <zstd.h>
#include "blosc2.h"
#include "blosc2/filters-registry.h"
#include "IlmThreadPool.h"
#include "ImfChannelList.h"
#include "ImfMisc.h"
namespace
{
/*
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
*/
class BloscInit
{
public:
    static void Init () { getInstance (); }
    BloscInit (const BloscInit&) = delete;
    BloscInit& operator= (const BloscInit&) = delete;
private:
    BloscInit () { blosc2_init (); }
    ~BloscInit () { blosc2_destroy (); }
    static BloscInit& getInstance ()
    {
        static BloscInit instance;
        return instance;
    }
};
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER
DeepCompressor::DeepCompressor ( const Header& hdr, size_t maxScanlineSize): Compressor(hdr),
    _maxScanlineSize (maxScanlineSize),_outBuffer (nullptr, &free), _schunk (nullptr,&blosc2_schunk_free)
{}

int
DeepCompressor::numScanLines () const
{
    return 1; // max ? // Needs to be in sync with ImfCompressor::numLinesInBuffer
}
int
DeepCompressor::compress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto ret = BLOSC_compress_impl(inPtr, inSize, outPtr);
    return ret;
}
int
DeepCompressor::uncompress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto ret = BLOSC_uncompress_impl(inPtr, inSize, outPtr);
    return ret;
}


int
DeepCompressor::BLOSC_compress_impl (const char* inPtr, int inSize, const char*& out)
{
    BloscInit::Init();
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;

    cparams.typesize = pixelTypeSize(PixelType::FLOAT); // Expect Float values
    cparams.clevel = header().zstdCompressionLevel();  // 9 is about a 20% increase in compression compared to 5. Decompression speed is unchanged.
    cparams.nthreads = 1;
    cparams.compcode = BLOSC_ZSTD; // Codec
    cparams.splitmode = BLOSC_NEVER_SPLIT;  // Split => multithreading, not split better compression


    blosc2_storage storage= BLOSC2_STORAGE_DEFAULTS;
    storage.cparams=&cparams;
    storage.contiguous = true;

    _schunk = schunk_ptr (blosc2_schunk_new(&storage), &blosc2_schunk_free);

    auto in = const_cast<char*>(inPtr);
    blosc2_schunk_append_buffer(_schunk.get(), in, inSize);

    uint8_t *buffer;
    bool shouldFree = true;
    auto size = blosc2_schunk_to_buffer(_schunk.get(), &buffer, &shouldFree);
    out = (char*)buffer;
    if (shouldFree)
    {
        _outBuffer = raw_ptr((char*)buffer, &free);
    }
    return size;
}

int
DeepCompressor::BLOSC_uncompress_impl (const char* inPtr, int inSize, const char*& out)
{
    auto in = const_cast<char*>(inPtr);
    _schunk = schunk_ptr (blosc2_schunk_from_buffer(reinterpret_cast<uint8_t*>(in), inSize, true), &blosc2_schunk_free);

    auto buffSize = _maxScanlineSize * numScanLines();
    _outBuffer = Imf::DeepCompressor::raw_ptr((char*)malloc (buffSize), &free);
    auto size = blosc2_schunk_decompress_chunk(_schunk.get(), 0, _outBuffer.get(), buffSize);
    out = _outBuffer.get();
    return size;
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT