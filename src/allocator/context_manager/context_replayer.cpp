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

#include "src/allocator/context_manager/context_replayer.h"

#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/context_manager/wbstripe_ctx/wbstripe_ctx.h"
#include "src/allocator/include/allocator_const.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ContextReplayer::ContextReplayer(AllocatorCtx* allocatorCtx_, SegmentCtx* segmentCtx_, WbStripeCtx* wbStripeCtx_, AllocatorAddressInfo* info_)
: allocatorCtx(allocatorCtx_),
  segmentCtx(segmentCtx_),
  wbStripeCtx(wbStripeCtx_),
  addrInfo(info_)
{
}

ContextReplayer::~ContextReplayer(void)
{
}

void
ContextReplayer::ResetDirtyContextVersion(int owner)
{
    if (owner == ALLOCATOR_CTX)
    {
        allocatorCtx->ResetDirtyVersion();
    }
    else if (owner == SEGMENT_CTX)
    {
        segmentCtx->ResetDirtyVersion();
    }
}

void
ContextReplayer::ReplaySsdLsid(StripeId currentSsdLsid)
{
    allocatorCtx->SetCurrentSsdLsid(currentSsdLsid);
}

void
ContextReplayer::ReplaySegmentAllocation(StripeId userLsid)
{
    SegmentId segId = userLsid / addrInfo->GetstripesPerSegment();
    if (segmentCtx->GetSegmentState(segId, false) == SegmentState::FREE)
    {
        segmentCtx->SetSegmentState(segId, SegmentState::NVRAM, false);
        allocatorCtx->AllocateSegment(segId);
        POS_TRACE_DEBUG((int)POS_EVENT_ID::JOURNAL_REPLAY_STATUS, "SegmentId:{} is allocated", segId);
    }
}

void
ContextReplayer::ReplayStripeAllocation(StripeId wbLsid, StripeId userLsid)
{
    assert(wbLsid != UNMAP_STRIPE);
    wbStripeCtx->AllocWbStripe(wbLsid);

    SegmentId segId = userLsid / addrInfo->GetstripesPerSegment();
    segmentCtx->SetSegmentState(segId, SegmentState::NVRAM, false);
}

void
ContextReplayer::ReplayStripeRelease(StripeId wbLsid)
{
    assert(wbLsid != UNMAP_STRIPE);
    wbStripeCtx->ReleaseWbStripe(wbLsid);
}

void
ContextReplayer::ReplayStripeFlushed(StripeId userLsid)
{
    // increase occupied stripe count
    SegmentId segId = userLsid / addrInfo->GetstripesPerSegment();
    bool segmentFreed = segmentCtx->IncreaseOccupiedStripeCount(segId);
    if (segmentFreed == true)
    {
        allocatorCtx->ReleaseSegment(segId);
        POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED), "segmentId:{} was freed by allocator", segId);
    }
}

void
ContextReplayer::ResetActiveStripeTail(int index)
{
    wbStripeCtx->SetActiveStripeTail(index, UNMAP_VSA);
}

std::vector<VirtualBlkAddr>
ContextReplayer::GetAllActiveStripeTail(void)
{
    return wbStripeCtx->GetAllActiveStripeTail();
}

void
ContextReplayer::ResetSegmentsStates(void)
{
    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        SegmentState state = segmentCtx->GetSegmentState(segId, false);
        if (state == SegmentState::VICTIM)
        {
            segmentCtx->SetSegmentState(segId, SegmentState::SSD, false);
            state = SegmentState::SSD;
            POS_TRACE_INFO(EID(SEGMENT_WAS_VICTIM), "segmentId:{} was VICTIM, so changed to SSD", segId);
        }
        if ((segmentCtx->GetValidBlockCount(segId) == 0) && (state == SegmentState::SSD))
        {
            segmentCtx->SetOccupiedStripeCount(segId, 0 /* count */);
            segmentCtx->SetSegmentState(segId, SegmentState::FREE, false);
            allocatorCtx->ReleaseSegment(segId);
            POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED), "segmentId:{} was All Invalidated, so changed to FREE", segId);
        }
    }
}

} // namespace pos
