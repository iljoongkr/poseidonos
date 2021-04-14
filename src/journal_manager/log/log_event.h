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

#include "src/allocator/context_manager/active_stripe_index_info.h"
#include "src/include/address_type.h"

namespace pos
{
static const uint32_t VALID_MARK = 0xCECECECE;

#pragma pack(push, 4)
enum class LogType
{
    BLOCK_WRITE_DONE,
    STRIPE_MAP_UPDATED,
    VOLUME_DELETED,
    NUM_LOG_TYPE
};

struct Log
{
    int mark = VALID_MARK;
    LogType type;
    uint32_t seqNum;
};

struct BlockWriteDoneLog : Log
{
    int volId;
    BlkAddr startRba;
    uint32_t numBlks;
    VirtualBlkAddr startVsa;
    int wbIndex;
    StripeAddr writeBufferStripeAddress;
    bool isGC;
    VirtualBlkAddr oldVsa;
};

struct StripeMapUpdatedLog : Log
{
    StripeId vsid;
    StripeAddr oldMap;
    StripeAddr newMap;
};

struct VolumeDeletedLog : Log
{
    int volId;
    uint64_t allocatorContextVersion;
};

#pragma pack(pop)

} // namespace pos
