//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

#include <cstring>
#include <mutex>
#include "openexr_compression.h"
#include "ImfZstdCompressor.h"


#include "IlmThreadPool.h"
#include "ImfChannelList.h"
#include "ImfMisc.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER

namespace
{
std::mutex g_mutex;
}

ZstdCompressor::ZstdCompressor (
    const Header& hdr, size_t maxScanlineSize, size_t numScanLines)
    : Compressor (hdr)
    , _numScanLines (numScanLines)
    , _outBuffer ()
{}

int
ZstdCompressor::numScanLines () const
{
    return _numScanLines; // Needs to be in sync with ImfCompressor::numLinesInBuffer
}

int
ZstdCompressor::compress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    outPtr = (char*) malloc (inSize);
    {
        std::lock_guard<std::mutex> lock (g_mutex);
        _outBuffer.push_back (raw_ptr ((char*) outPtr, &free));
    }
    auto fullSize = BLOSC_compress_impl (
        (char*) (inPtr), inSize, (void*)outPtr, inSize);
    return fullSize;
}

int
ZstdCompressor::uncompress (
    const char* inPtr, int inSize, int minY, const char*& outPtr)
{
    auto read = (const char*) inPtr;
    void* write = nullptr;
    auto ret = BLOSC_uncompress_impl_single_blob (
            read, inSize, &write, 0);
    {
        std::lock_guard<std::mutex> lock (g_mutex);
        _outBuffer.push_back (raw_ptr ((char*) write, &free));
    }
    outPtr = (const char*)write;
    return ret;

}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT