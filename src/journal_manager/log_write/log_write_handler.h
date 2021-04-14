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

#include "../log/waiting_log_list.h"
#include "../log_buffer/buffer_write_done_notifier.h"
#include "src/meta_file_intf/async_context.h"

namespace pos
{
class BufferOffsetAllocator;
class LogWriteContext;
class JournalLogBuffer;
class JournalConfiguration;
class LogWriteStatistics;

class LogWriteHandler : public LogBufferWriteDoneEvent
{
public:
    LogWriteHandler(void);
    LogWriteHandler(LogWriteStatistics* statistics, WaitingLogList* waitingList);
    virtual ~LogWriteHandler(void);

    virtual void Init(BufferOffsetAllocator* allocator, JournalLogBuffer* buffer,
        JournalConfiguration* config);

    virtual int AddLog(LogWriteContext* context);
    void LogWriteDone(AsyncMetaFileIoCtx* ctx);

    virtual void LogFilled(int logGroupId, MapPageList& dirty) override;
    virtual void LogBufferReseted(int logGroupId) override;

private:
    int _AddLogInternal(LogWriteContext* context);
    void _StartWaitingIos(void);

    JournalLogBuffer* logBuffer;
    BufferOffsetAllocator* bufferAllocator;

    LogWriteStatistics* logWriteStats;
    WaitingLogList* waitingList;
};

} // namespace pos
