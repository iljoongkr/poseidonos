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

#include "src/allocator/include/allocator_const.h"
#include "src/volume/volume_list.h"

namespace pos
{
struct ActiveStripeTailArrIdxInfo
{
    ActiveStripeTailArrIdxInfo(void) = default;
    ActiveStripeTailArrIdxInfo(uint32_t volId, bool gc)
    {
        volumeId = volId;
        isGc = gc;
    }

    ASTailArrayIdx GetActiveStripeTailArrIdx(void)
    {
        if (isGc)
        {
            return MAX_VOLUME_COUNT + volumeId;
        }
        else
        {
            return volumeId;
        }
    }

    bool
    operator==(ActiveStripeTailArrIdxInfo input) const
    {
        return (volumeId == input.volumeId && isGc == input.isGc);
    }

    static ASTailArrayIdx GetVolumeId(ASTailArrayIdx index)
    {
        if (index > MAX_VOLUME_COUNT)
        {
            return index - MAX_VOLUME_COUNT;
        }
        else
        {
            return index;
        }
    }

    static bool IsGc(ASTailArrayIdx index)
    {
        return (index >= MAX_VOLUME_COUNT);
    }

    static const ASTailArrayIdx INVALID_INDEX = 0xFFFFFFFF;

    uint32_t volumeId;
    bool isGc;
};

} // namespace pos
