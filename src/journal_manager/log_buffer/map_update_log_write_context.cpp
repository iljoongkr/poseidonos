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

#include "src/journal_manager/log_buffer/map_update_log_write_context.h"

#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/journal_manager/log_buffer/callback_sequence_controller.h"

namespace pos
{
MapUpdateLogWriteContext::MapUpdateLogWriteContext(MapPageList dirtyList, EventSmartPtr callback,
    LogBufferWriteDoneNotifier* logFilledNotifier, CallbackSequenceController* sequencer)
: LogWriteContext(callback, logFilledNotifier),
  sequenceController(sequencer),
  dirty(dirtyList)
{
}

MapUpdateLogWriteContext::MapUpdateLogWriteContext(LogHandlerInterface* log,
    MapPageList dirtyList, EventSmartPtr callback,
    LogBufferWriteDoneNotifier* logFilledNotifier,
    CallbackSequenceController* sequencer)
: LogWriteContext(log, callback, logFilledNotifier),
  sequenceController(sequencer),
  dirty(dirtyList)
{
}

MapPageList&
MapUpdateLogWriteContext::GetDirtyList(void)
{
    return dirty;
}

void
MapUpdateLogWriteContext::IoDone(void)
{
    sequenceController->GetCallbackExecutionApproval();
    LogBufferIoContext::IoDone();
    sequenceController->NotifyCallbackCompleted();

    // Log filled notify should be after the callback function completed
    logFilledNotifier->NotifyLogFilled(GetLogGroupId(), dirty);
}

} // namespace pos
