#pragma once
#include <memory>
#include "ImfNamespace.h"
#include "ImfCompressor.h"


OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER
class ZstdCompressor: public Compressor
{
    using raw_ptr = std::unique_ptr<char, decltype(&free)> ;
    raw_ptr _outBuffer;
    int _maxScanLineSize;
    int numScanLines () const override;
    int compress (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;
    int uncompress (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;

public:
    ZstdCompressor (const Header& hdr, int maxScanLineSize);
};

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT