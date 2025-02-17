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

#pragma once

#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/block_allocation_status.h"
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_context_replayer.h"
#include "src/allocator/include/allocator_const.h"
#include "src/state/interface/i_state_control.h"

namespace pos
{
class IAllocatorFileIoClient;
class AllocatorFileIoManager;
class AllocatorCtx;
class SegmentCtx;
class WbStripeCtx;
class ContextReplayer;
class TelemetryPublisher;
class EventScheduler;

const int NO_REBUILD_TARGET_USER_SEGMENT = 0;

class ContextManager : public IContextManager
{
public:
    enum IOTYPE
    {
        IOTYPE_READ,
        IOTYPE_FLUSH,
        IOTYPE_REBUILDFLUSH,
        IOTYPE_ALL
    };

    ContextManager(void) = default;
    ContextManager(TelemetryPublisher* tp,
        AllocatorCtx* allocCtx_, SegmentCtx* segCtx_, RebuildCtx* rebuildCtx_,
        WbStripeCtx* wbstripeCtx_, GcCtx* gcCtx_, BlockAllocationStatus* blockAllocStatus_,
        AllocatorFileIoManager* fileMananager_,
        ContextReplayer* ctxReplayer_, bool flushProgress, AllocatorAddressInfo* info_, uint32_t arrayId_);
    ContextManager(TelemetryPublisher* tp, EventScheduler* eventScheduler_,
        AllocatorCtx* allocCtx_, SegmentCtx* segCtx_, RebuildCtx* rebuildCtx_,
        WbStripeCtx* wbstripeCtx_, GcCtx* gcCtx_, BlockAllocationStatus* blockAllocStatus_,
        AllocatorFileIoManager* fileMananager_,
        ContextReplayer* ctxReplayer_, bool flushProgress, AllocatorAddressInfo* info_, uint32_t arrayId_);
    ContextManager(TelemetryPublisher* tp, AllocatorAddressInfo* info, uint32_t arrayId_);
    virtual ~ContextManager(void);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual int FlushContexts(EventSmartPtr callback, bool sync);
    virtual int FlushRebuildContext(EventSmartPtr callback, bool sync);
    virtual void UpdateOccupiedStripeCount(StripeId lsid);
    virtual SegmentId AllocateFreeSegment(void);
    virtual SegmentId AllocateGCVictimSegment(void);
    virtual SegmentId AllocateRebuildTargetSegment(void);
    virtual int ReleaseRebuildSegment(SegmentId segmentId);
    virtual bool NeedRebuildAgain(void);
    virtual int MakeRebuildTarget(void);
    virtual int StopRebuilding(void);
    virtual uint32_t GetRebuildTargetSegmentCount(void);
    virtual int GetNumOfFreeSegment(bool needLock);
    virtual GcMode GetCurrentGcMode(void);
    virtual int GetGcThreshold(GcMode mode);
    virtual uint64_t GetStoredContextVersion(int owner);

    virtual void IncreaseValidBlockCount(SegmentId segId, uint32_t count);
    virtual void DecreaseValidBlockCount(SegmentId segId, uint32_t count);

    virtual int SetNextSsdLsid(void);
    virtual char* GetContextSectionAddr(int owner, int section);
    virtual int GetContextSectionSize(int owner, int section);

    virtual RebuildCtx* GetRebuildCtx(void) { return rebuildCtx; }
    virtual SegmentCtx* GetSegmentCtx(void) { return segmentCtx; }
    virtual AllocatorCtx* GetAllocatorCtx(void) { return allocatorCtx; }
    virtual WbStripeCtx* GetWbStripeCtx(void) { return wbStripeCtx; }
    virtual ContextReplayer* GetContextReplayer(void) { return contextReplayer; }
    virtual GcCtx* GetGcCtx(void) { return gcCtx; }
    virtual std::mutex& GetCtxLock(void) { return ctxLock; }

    virtual BlockAllocationStatus* GetAllocationStatus(void) { return blockAllocStatus; }

    // for UT
    void SetCallbackFunc(EventSmartPtr callback);
    void TestCallbackFunc(AsyncMetaFileIoCtx* ctx, IOTYPE type, int cnt);

private:
    void _UpdateSectionInfo(void);
    int _LoadContexts(void);
    int _Flush(int owner);
    void _LoadCompletedThenCB(AsyncMetaFileIoCtx* ctx);
    void _FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx);
    void _RebuildFlushCompletedThenCB(AsyncMetaFileIoCtx* ctx);
    void _WaitPendingIo(IOTYPE type);
    MetaIoCbPtr _SetCallbackFunc(int owner, EventSmartPtr callbackEvent);
    void _PrepareBuffer(int owner, char* buf);
    void _ResetSegmentStates(void);
    void _NotifySegmentFreed(SegmentId segId);

    std::string fileNames[NUM_FILES] = {"SegmentContext", "AllocatorContexts", "RebuildContext"};
    IAllocatorFileIoClient* fileOwner[NUM_FILES];
    std::atomic<int> numReadIoIssued;
    std::atomic<int> numFlushIoIssued;
    std::atomic<int> numRebuildFlushIoIssued;
    std::atomic<bool> flushInProgress;
    EventSmartPtr flushCallback;

    AllocatorFileIoManager* fileIoManager;
    AllocatorAddressInfo* addrInfo;
    AllocatorCtx* allocatorCtx;
    SegmentCtx* segmentCtx;
    WbStripeCtx* wbStripeCtx;
    RebuildCtx* rebuildCtx;
    ContextReplayer* contextReplayer;
    GcCtx* gcCtx;
    GcMode curGcMode;
    GcMode prevGcMode;
    BlockAllocationStatus* blockAllocStatus;

    uint32_t arrayId;
    std::mutex ctxLock;

    TelemetryPublisher* telPublisher;
    EventScheduler* eventScheduler;
};

} // namespace pos
