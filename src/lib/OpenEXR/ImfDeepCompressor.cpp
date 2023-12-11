//
// Created by vladal on 27/10/23.
//

#include <cstring>
#include "ImfDeepCompressor.h"
#include "IlmThread.h"
//#include <zstd.h>
#include "blosc2.h"
#include "blosc2/filters-registry.h"
#include "blosc2/codecs-registry.h"
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

blosc2_cparams getBloscCParams(Imf::PixelType type)
{
    auto cparams = BLOSC2_CPARAMS_DEFAULTS;

    cparams.typesize = pixelTypeSize (type); // Expect Float values
    cparams.clevel =
        5; // 9 is about a 20% increase in compression compared to 5. Decompression speed is unchanged.
    cparams.nthreads =1;
    cparams.compcode = BLOSC_ZSTD; // Codec
    cparams.splitmode = BLOSC_NEVER_SPLIT;
    //ILMTHREAD_NAMESPACE::ThreadPool::globalThreadPool ().numThreads();
    switch (type)
    {
        case Imf::PixelType::HALF:
            break;
        case Imf::PixelType::FLOAT:
            cparams.compcode = BLOSC_CODEC_ZFP_FIXED_ACCURACY;
            cparams.compcode_meta = -5;
            cparams.filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_NOFILTER;
            break;
        case Imf::PixelType::UINT:
            cparams.use_dict = true; // Codec
            break;
    }


    return cparams;
}


class BloscInit
{
public:
    static void Init () { getInstance (); }
    BloscInit (const BloscInit&)            = delete;
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
} // namespace

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER
DeepCompressor::DeepCompressor (const Header& hdr, size_t maxScanlineSize)
    : Compressor (hdr)
    , _maxScanlineSize (maxScanlineSize)
  //  , _outBuffer (nullptr, &free)
  //  , _intermediateBuffer (nullptr, &free)
   // , _schunk (nullptr, &blosc2_schunk_free)
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
    //should never reach
    //auto ret = BLOSC_compress_impl (inPtr, inSize, outPtr);
    return 0;
}
int
DeepCompressor::uncompress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    //auto ret = BLOSC_uncompress_impl (inPtr, inSize, outPtr);
    return 0;
}

int
DeepCompressor::BLOSC_compress_impl (
    const char* inPtr, int inSize, PixelType type, const char*& out)
{
    BloscInit::Init ();
    blosc2_cparams cparams = getBloscCParams(type);
    
    blosc2_storage storage = BLOSC2_STORAGE_DEFAULTS;
    storage.cparams        = &cparams;
    storage.contiguous     = true;

    auto schunk = schunk_ptr (blosc2_schunk_new (&storage), &blosc2_schunk_free);

    auto in = const_cast<char*> (inPtr);
    auto nChunks = blosc2_schunk_append_buffer (schunk.get (), in, inSize);

    uint8_t* buffer;
    bool     shouldFree = true;
    auto size = blosc2_schunk_to_buffer (schunk.get (), &buffer, &shouldFree);
    out       = (char*) buffer;
    if (shouldFree) { _outBuffers.push_back ( raw_ptr ((char*) buffer, &free)); }
    _schunks.push_back (std::move (schunk));
    return size;
}

int
DeepCompressor::BLOSC_uncompress_impl (
    const char* inPtr, int inSize, const char*& out)
{
    auto in = const_cast<char*> (inPtr);
    auto schunk = schunk_ptr (
        blosc2_schunk_from_buffer (
            reinterpret_cast<uint8_t*> (in), inSize, true),
        &blosc2_schunk_free);

    auto buffSize = _maxScanlineSize * numScanLines ();
    auto outBuffer =
        Imf::DeepCompressor::raw_ptr ((char*) malloc (buffSize), &free);
    auto size = blosc2_schunk_decompress_chunk (
        schunk.get (), 0, outBuffer.get (), buffSize);
    out = outBuffer.get ();

    _schunks.push_back (std::move (schunk));
    _outBuffers.push_back (std::move (outBuffer));
    return size;
}
int
DeepCompressor::compressSampleCountTable (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto ret = BLOSC_compress_impl (inPtr, inSize, PixelType::UINT,outPtr);
    return ret;
}
int
DeepCompressor::uncompressSampleCountTable (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto ret = BLOSC_uncompress_impl (inPtr, inSize, outPtr);
    return ret;
}
/*
Compressor::CompressorDataContext
DeepCompressor::interlace (const Compressor::CompressorDataContext& ctx)
{
    _intermediateBuffer = raw_ptr ((char*) malloc (ctx.inSize), &free);

    auto ret  = ctx;
    ret.inPtr = _intermediateBuffer.get ();
    memcpy ((void*) ret.inPtr, ctx.inPtr, ctx.inSize);
    return ret;
}*/
/*
Compressor::CompressorDataContext
DeepCompressor::deinterlace (const Compressor::CompressorDataContext& ctx)
{
   // _intermediateBuffer = raw_ptr ((char*) malloc (ctx.inSize), &free);

    const char* in  = (char*) ctx.inPtr;
    //char*       out = (char*) _intermediateBuffer.get ();

    auto start = in;
    for (auto c = header ().channels ().begin ();
         c != header ().channels ().end ();
         ++c)
    {
        auto layout = DataLayout{start,  ctx.samplesStrideArray[ctx.samplesStrideArraySize-1] , c.channel ().type};
        _dataLayout.push_back (layout);
        auto bytes = ctx.samplesStrideArray[ctx.samplesStrideArraySize-1] * pixelTypeSize (c.channel ().type);
        start += bytes;
    }

    for (auto l = _dataLayout.begin();l!=_dataLayout.end();++l)
    {
        auto buff = l->start;
        for (int i=0;i<l->numElements;i++)
        {
            switch (l->type)
            {
                case PixelType::HALF: {
                    half h;
                    Xdr::read<CharPtrIO> (buff, h);
                    break;
                }
                case PixelType::FLOAT: {
                    float f;
                    Xdr::read<CharPtrIO> (buff, f);
                    //std::cout<<"i-f"<<i<<"||||"<<f<<std::endl;
                    break;
                }
                case PixelType::UINT: {
                    unsigned int ui;
                    Xdr::read<CharPtrIO> (buff, ui);
                    break;
                }
            }
        }
    }


    auto ret  = ctx;
    //ret.inPtr = _intermediateBuffer.get ();
    memcpy ((void*) ret.inPtr, ctx.inPtr, ctx.inSize);
    return ret;
}*/
int
DeepCompressor::compress (
    const Compressor::CompressorDataContext& ctx, const char*& outPtr)
{
    //auto intermediate = deinterlace (ctx);
    const char* in  = (char*) ctx.inPtr;

    std::vector<std::pair<const char*, int>> buffs;
    auto start = in;
    int totalSize = 0;
    for (auto c = header ().channels ().begin ();
         c != header ().channels ().end ();
         ++c)
    {
        auto type = c.channel ().type;
        auto bytes = ctx.samplesStrideArray[ctx.samplesStrideArraySize-1] * pixelTypeSize (c.channel ().type);

        const char *channelBuff;
        auto newSize = BLOSC_compress_impl (start, bytes, type, channelBuff);
        buffs.push_back (std::make_pair (channelBuff, newSize));
        totalSize+= newSize;

        start += bytes;
    }

   /* for (auto l = _dataLayout.begin();l!=_dataLayout.end();++l)
    {
        auto buff = l->start;
        for (int i=0;i<l->numElements;i++)
        {
            switch (l->type)
            {
                case PixelType::HALF: {
                    half h;
                    Xdr::read<CharPtrIO> (buff, h);
                    break;
                }
                case PixelType::FLOAT: {
                    float f;
                    Xdr::read<CharPtrIO> (buff, f);
                    //std::cout<<"i-f"<<i<<"||||"<<f<<std::endl;
                    break;
                }
                case PixelType::UINT: {
                    unsigned int ui;
                    Xdr::read<CharPtrIO> (buff, ui);
                    break;
                }
            }
        }
    }*/

    totalSize += computeSerializationOverheadSize();
    auto outBuffer = raw_ptr ((char*) malloc (totalSize), &free);

    auto out = outBuffer.get ();
    Xdr::write<CharPtrIO> (out, (int)CodecVersion::V0);

    for (int i=0;i<buffs.size();i++)
    {
        Xdr::write<CharPtrIO> (out, buffs[i].second);
        auto src = (void*)buffs[i].first;
        auto dst = (void*)(out);
        memcpy (dst, src, buffs[i].second);
        out+=buffs[i].second;
    }

    outPtr = outBuffer.get ();

    _outBuffers.clear();

    _outBuffers.push_back (std::move (outBuffer));
    return totalSize;
}
int
DeepCompressor::uncompress (
    const Compressor::CompressorDataContext& ctx, const char*& outPtr)
{
    auto in= ctx.inPtr;
    int magicNumber;
    Xdr::read<CharPtrIO>(in, magicNumber);
    if (magicNumber != CodecVersion::V0)
    {
        // Error.
        return 0;
    }
    int totalSize = 0;
    std::vector<std::pair<const char*, int>> buffs;
    //while (in <= ctx.inPtr + ctx.inSize)
    for (auto c = header ().channels ().begin ();
         c != header ().channels ().end ();
         ++c)
    {
        int size;
        Xdr::read<CharPtrIO>(in, size);
        const char* out;
        auto newSize =BLOSC_uncompress_impl(in, size, out);
        buffs.push_back (std::make_pair (out, newSize));

        in+=size;
        totalSize+= newSize;
        /*auto channelBuff = (char*)malloc (size);
        auto src = (void*)in;
        auto dst = (void*)channelBuff;
        memcpy (dst, src, size);
        in+=size;
        _outBuffers.push_back (raw_ptr (channelBuff, &free));*/
    }

    outPtr =  (const char*)malloc (totalSize);
    auto wrtPtr = (char*)outPtr;
    for (int i=0;i<buffs.size();i++)
    {
        auto src = (void*)buffs[i].first;
        auto dst = (void*)(wrtPtr);
        memcpy (dst, src, buffs[i].second);
        wrtPtr+=buffs[i].second;
    }

    _outBuffers.clear();
    _outBuffers.push_back (raw_ptr ((char*)outPtr, &free));
    return totalSize;//Compressor::uncompress (intermediate, outPtr);
}
int
DeepCompressor::computeSerializationOverheadSize () const
{
    int i=0;
    for (auto c = header ().channels ().begin ();
         c != header ().channels ().end ();
         ++c, ++i)
    {}
    return Xdr::size<int> () + i *Xdr::size<int> (); // magic number|size1|bytes1|size2|bytes2|...
}
DeepCompressor::~DeepCompressor ()
{
    _schunks.clear ();
    _outBuffers.clear ();
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT