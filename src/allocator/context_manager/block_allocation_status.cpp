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

#include "src/allocator/context_manager/block_allocation_status.h"

namespace pos
{
BlockAllocationStatus::BlockAllocationStatus(void)
: userBlkAllocProhibited(false)
{
    for (int volume = 0; volume < MAX_VOLUME_COUNT; volume++)
    {
        blkAllocProhibited[volume] = false;
    }
}

bool
BlockAllocationStatus::IsUserBlockAllocationProhibited(int volumeId)
{
    return ((blkAllocProhibited[volumeId] == true) || (userBlkAllocProhibited == true));
}

bool
BlockAllocationStatus::IsBlockAllocationProhibited(int volumeId)
{
    return blkAllocProhibited[volumeId];
}

void
BlockAllocationStatus::PermitUserBlockAllocation(void)
{
    userBlkAllocProhibited = false;
}

void
BlockAllocationStatus::PermitBlockAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        blkAllocProhibited[i] = false;
    }
}

void
BlockAllocationStatus::PermitBlockAllocation(int volumeId)
{
    blkAllocProhibited[volumeId] = false;
}

void
BlockAllocationStatus::ProhibitUserBlockAllocation(void)
{
    userBlkAllocProhibited = true;
}

void
BlockAllocationStatus::ProhibitBlockAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        // Wait for flag to be reset
        while (blkAllocProhibited[i].exchange(true) == true)
        {
        }
    }
}

bool
BlockAllocationStatus::TryProhibitBlockAllocation(int volumeId)
{
    return (blkAllocProhibited[volumeId].exchange(true) == false);
}
} // namespace pos
