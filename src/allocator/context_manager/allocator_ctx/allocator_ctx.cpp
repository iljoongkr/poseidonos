/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.h"
#include "src/lib/bitmap.h"
#include "src/logger/logger.h"

namespace pos
{
AllocatorCtx::AllocatorCtx(AllocatorCtxHeader* header, BitMapMutex* allocSegBitmap_, AllocatorAddressInfo* info_)
: ctxStoredVersion(0),
  ctxDirtyVersion(0),
  addrInfo(info_),
  initialized(false)
{
    allocSegBitmap = allocSegBitmap_; // for UT

    ctxHeader.sig = SIG_ALLOCATOR_CTX;
    ctxHeader.numValidSegment = 0;
    ctxHeader.numValidWbLsid = 0;
    ctxHeader.ctxVersion = 0;
    prevSsdLsid = 0;
    currentSsdLsid = 0;

    if (header != nullptr)
    {
        ctxHeader.sig = header->sig;
        ctxHeader.numValidSegment = header->numValidSegment;
        ctxHeader.numValidWbLsid = header->numValidWbLsid;
        ctxHeader.ctxVersion = header->ctxVersion;
    }
}

AllocatorCtx::AllocatorCtx(AllocatorAddressInfo* info)
: AllocatorCtx(nullptr, nullptr, info)
{
}

AllocatorCtx::~AllocatorCtx(void)
{
    Dispose();
}

void
AllocatorCtx::Init(void)
{
    if (initialized == true)
    {
        return;
    }


    uint32_t numSegment = addrInfo->GetnumUserAreaSegments();
    if (allocSegBitmap == nullptr)
    {
        allocSegBitmap = new BitMapMutex(numSegment);
    }
    currentSsdLsid = STRIPES_PER_SEGMENT - 1;
    prevSsdLsid = STRIPES_PER_SEGMENT - 1;

    ctxHeader.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;
    initialized = true;
}

void
AllocatorCtx::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (allocSegBitmap != nullptr)
    {
        delete allocSegBitmap;
        allocSegBitmap = nullptr;
    }
    initialized = false;
}

void
AllocatorCtx::AllocateSegment(SegmentId segId)
{
    allocSegBitmap->SetBit(segId);
}

void
AllocatorCtx::ReleaseSegment(SegmentId segId)
{
    allocSegBitmap->ClearBit(segId);
}

SegmentId
AllocatorCtx::AllocateFreeSegment(SegmentId startSegId)
{
    SegmentId segId;
    if (startSegId == UNMAP_SEGMENT)
    {
        segId = allocSegBitmap->SetNextZeroBit();
    }
    else
    {
        segId = allocSegBitmap->SetFirstZeroBit(startSegId);
    }

    if (allocSegBitmap->IsValidBit(segId) == false)
    {
        segId = UNMAP_SEGMENT;
    }
    return segId;
}

SegmentId
AllocatorCtx::GetUsedSegment(SegmentId startSegId)
{
    SegmentId segId = allocSegBitmap->FindFirstSetBit(startSegId);
    if (allocSegBitmap->IsValidBit(segId) == false)
    {
        segId = UNMAP_SEGMENT;
    }
    return segId;
}

void
AllocatorCtx::SetNextSsdLsid(SegmentId segId)
{
    prevSsdLsid = currentSsdLsid;
    currentSsdLsid = segId * addrInfo->GetstripesPerSegment();
}

StripeId
AllocatorCtx::GetPrevSsdLsid(void)
{
    return prevSsdLsid;
}

void
AllocatorCtx::SetPrevSsdLsid(StripeId stripeId)
{
    prevSsdLsid = stripeId;
}

StripeId
AllocatorCtx::UpdatePrevLsid(void)
{
    prevSsdLsid = currentSsdLsid;
    return currentSsdLsid + 1;
}

void
AllocatorCtx::SetCurrentSsdLsid(StripeId stripe)
{
    currentSsdLsid = stripe;
}

StripeId
AllocatorCtx::GetCurrentSsdLsid(void)
{
    return currentSsdLsid;
}

void
AllocatorCtx::RollbackCurrentSsdLsid(void)
{
    currentSsdLsid = prevSsdLsid;
}

uint64_t
AllocatorCtx::GetNumOfFreeSegment(void)
{
    return allocSegBitmap->GetNumBits() - allocSegBitmap->GetNumBitsSet();
}

uint64_t
AllocatorCtx::GetNumOfFreeSegmentWoLock(void)
{
    return allocSegBitmap->GetNumBits() - allocSegBitmap->GetNumBitsSetWoLock();
}

void
AllocatorCtx::SetAllocatedSegmentCount(int count)
{
    allocSegBitmap->SetNumBitsSet(count);
}

int
AllocatorCtx::GetAllocatedSegmentCount(void)
{
    return allocSegBitmap->GetNumBitsSet();
}

int
AllocatorCtx::GetTotalSegmentsCount(void)
{
    return allocSegBitmap->GetNumBits();
}

void
AllocatorCtx::AfterLoad(char* buf)
{
    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "AllocatorCtx file loaded:{}, {}", ctxHeader.ctxVersion, ctxHeader.numValidSegment);
    allocSegBitmap->SetNumBitsSet(ctxHeader.numValidSegment);
    ctxDirtyVersion = ctxHeader.ctxVersion + 1;
}

void
AllocatorCtx::BeforeFlush(int section, char* buf)
{
    ctxHeader.ctxVersion = ctxDirtyVersion++;
    ctxHeader.numValidSegment = allocSegBitmap->GetNumBitsSet();
}

void
AllocatorCtx::FinalizeIo(AsyncMetaFileIoCtx* ctx)
{
    ctxStoredVersion = ((AllocatorCtxHeader*)ctx->buffer)->ctxVersion;
}

char*
AllocatorCtx::GetSectionAddr(int section)
{
    char* ret = nullptr;
    switch (section)
    {
        case AC_HEADER:
        {
            ret = (char*)&ctxHeader;
            break;
        }
        case AC_ALLOCATE_SEGMENT_BITMAP:
        {
            ret = (char*)allocSegBitmap->GetMapAddr();
            break;
        }
        case AC_CURRENT_SSD_LSID:
        {
            ret = (char*)&currentSsdLsid;
            break;
        }
    }
    return ret;
}

int
AllocatorCtx::GetSectionSize(int section)
{
    int ret = 0;
    switch (section)
    {
        case AC_HEADER:
        {
            ret = sizeof(AllocatorCtxHeader);
            break;
        }
        case AC_ALLOCATE_SEGMENT_BITMAP:
        {
            ret = allocSegBitmap->GetNumEntry() * BITMAP_ENTRY_SIZE;
            break;
        }
        case AC_CURRENT_SSD_LSID:
        {
            ret = sizeof(currentSsdLsid);
            break;
        }
    }
    return ret;
}

uint64_t
AllocatorCtx::GetStoredVersion(void)
{
    return ctxStoredVersion;
}

void
AllocatorCtx::ResetDirtyVersion(void)
{
    ctxDirtyVersion = 0;
}

}  // namespace pos
