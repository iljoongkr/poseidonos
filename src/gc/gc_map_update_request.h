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

#include <atomic>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "gc_map_update_completion.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event.h"
#include "src/include/address_type.h"
#include "src/journal_manager/log/gc_map_update_list.h"
#include "src/journal_service/journal_service.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/include/mpage_info.h"
#include "src/meta_service/i_meta_updater.h"

namespace pos
{
class Stripe;
class IStripeMap;
class EventScheduler;
class GcStripeManager;
class IArrayInfo;

class GcMapUpdateRequest : public Event
{
public:
    explicit GcMapUpdateRequest(Stripe* stripe, std::string arrayName, GcStripeManager* gcStripeManager);
    GcMapUpdateRequest(Stripe* stripe,
        CallbackSmartPtr completionEvent,
        IVSAMap* inputIVSAMap,
        IArrayInfo* inputIArrayInfo,
        IMetaUpdater* inputMetaUpdater);
    virtual bool Execute(void) override;

private:
    bool _BuildMeta(void);
    bool _UpdateMeta(void);
    void _AddBlockMapUpdateLog(BlkAddr rba, VirtualBlkAddr writeVsa);
    void _GetDirtyPages(uint32_t volId, BlkAddr rba);
    void _AddVsaMapUpdateLog(uint32_t volId, BlkAddr rba, VirtualBlks writeVsaRange);
    void _RegisterInvalidateSegments(VirtualBlkAddr vsa);

    Stripe* stripe;

    CallbackSmartPtr completionEvent;
    IVSAMap* iVSAMap;
    IArrayInfo* iArrayInfo;
    IMetaUpdater* metaUpdater;

    uint32_t totalBlksPerUserStripe;
    uint32_t stripesPerSegment;
    uint32_t currentStripeOffset;

    std::map<SegmentId, uint32_t > invalidSegCnt;

    MpageList volumeDirtyList;
    GcStripeMapUpdateList mapUpdates;
};
} // namespace pos

