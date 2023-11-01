#pragma once
#include <memory>
#include "ImfNamespace.h"
#include "ImfCompressor.h"


OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER
class ZstdCompressor: public Compressor
{
    using raw_ptr = std::unique_ptr<char, decltype(&free)> ;
    raw_ptr _outBuffer;
    size_t  _maxScanLines;
    int numScanLines () const override; // max
    int compress (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;
    int uncompress (
        const char* inPtr, int inSize, int minY, const char*& outPtr) override;

public:
    ZstdCompressor (const Header& hdr, size_t maxScanLines);
};

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT