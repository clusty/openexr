#pragma once
#include <memory>
#include <vector>
#include "ImfNamespace.h"
#include "ImfCompressor.h"
#include "ImfHeader.h"
#include "blosc2.h"
#include "ImfPixelType.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER
class DeepCompressor : public Compressor
{
public:
    DeepCompressor (const Header& hdr, size_t maxScanLines);
    ~DeepCompressor () override;

private:
    using schunk_ptr = std::unique_ptr<blosc2_schunk, decltype(&blosc2_schunk_free)> ;
    using raw_ptr = std::unique_ptr<char, decltype(&free)> ;

    std::vector<schunk_ptr> _schunks;
    std::vector<raw_ptr> _outBuffers;

    size_t  _maxScanlineSize;
    int numScanLines () const override; // max
    int compress (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;
    int uncompress (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;
    int compress (
        const CompressorDataContext& ctx, const char*& outPtr) override;
    int uncompress (
        const CompressorDataContext& ctx, const char*& outPtr) override;

    int compressSampleCountTable (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;
    int uncompressSampleCountTable (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;

    int BLOSC_compress_impl (const char* inPtr, int inSize, PixelType type, const char*& out);
    int BLOSC_uncompress_impl (const char* inPtr, int inSize, const char*& out);
    //CompressorDataContext deinterlace(const CompressorDataContext& ctx);
    //CompressorDataContext interlace(const CompressorDataContext& ctx);

    int computeSerializationOverheadSize() const;


    enum CodecVersion : int
    {
        V0 = 0,

    };
};

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT