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

#include <cstdint>
#include <string>

#include "src/event_scheduler/callback.h"
#include "src/include/address_type.h"
#include "src/include/pos_event_id.hpp"
#include "src/io_submit_interface/io_submit_handler_status.h"
#include "src/state/state_context.h"

namespace pos
{
class BufferEntry;
class IODispatcher;

class MergedIO
{
public:
    explicit MergedIO(CallbackSmartPtr callback, IODispatcher* ioDispatcher = nullptr, StateType stateType = StateEnum::TYPE_COUNT);
    ~MergedIO(void);

    void Reset(void);
    IOSubmitHandlerStatus Process(int arrayId);
    void AddContiguousBlock(void);
    void SetNewStart(void* newBuffer, PhysicalBlkAddr& newPba);
    bool IsContiguous(PhysicalBlkAddr& targetPba);

private:
    BufferEntry* bufferEntry;
    PhysicalBlkAddr startPba;
    uint64_t nextContiguousLba;
    CallbackSmartPtr callback;
    IODispatcher* ioDispatcher;
    StateType stateType;

    IOSubmitHandlerStatus _CheckAsyncReadError(POS_EVENT_ID eventId, int arrayId);
};
} // namespace pos
