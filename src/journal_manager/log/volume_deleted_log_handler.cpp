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

#include "src/journal_manager/log/volume_deleted_log_handler.h"

namespace pos
{
VolumeDeletedLogEntry::VolumeDeletedLogEntry(int volId, uint64_t contextVersion)
{
    dat.type = LogType::VOLUME_DELETED;
    dat.volId = volId;
    dat.allocatorContextVersion = contextVersion;
}

VolumeDeletedLogEntry::VolumeDeletedLogEntry(VolumeDeletedLog& log)
{
    dat = log;
}

bool
VolumeDeletedLogEntry::operator==(const VolumeDeletedLogEntry log)
{
    return ((log.dat.volId == dat.volId) &&
        (log.dat.allocatorContextVersion == dat.allocatorContextVersion));
}

LogType
VolumeDeletedLogEntry::GetType(void)
{
    return dat.type;
}

uint32_t
VolumeDeletedLogEntry::GetSize(void)
{
    return sizeof(VolumeDeletedLog);
}

char*
VolumeDeletedLogEntry::GetData(void)
{
    return (char*)&dat;
}

StripeId
VolumeDeletedLogEntry::GetVsid(void)
{
    // This is invalid operation
    return UNMAP_STRIPE;
}

uint32_t
VolumeDeletedLogEntry::GetSeqNum(void)
{
    return dat.seqNum;
}

void
VolumeDeletedLogEntry::SetSeqNum(uint32_t num)
{
    dat.seqNum = num;
}

} // namespace pos
