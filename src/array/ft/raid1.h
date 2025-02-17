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

#ifndef RAID1_H_
#define RAID1_H_

#include <list>
#include <vector>

#include "method.h"

namespace pos
{
class PartitionPhysicalSize;
class RebuildBehavior;

class Raid1 : public Method
{
public:
    Raid1(const PartitionPhysicalSize* physicalSize);
    virtual ~Raid1();

    virtual int Translate(FtBlkAddr&, const LogicalBlkAddr&) override;
    virtual int Convert(list<FtWriteEntry>&, const LogicalWriteEntry&) override;
    list<FtBlkAddr> GetRebuildGroup(FtBlkAddr fba) override;
    virtual RaidState GetRaidState(vector<ArrayDeviceState> devs) override;

private:
    void _RebuildData(void* dst, void* src, uint32_t size);
    uint32_t _GetMirrorIndex(uint32_t idx);
    virtual void _BindRecoverFunc(void) override;
    uint32_t mirrorDevCnt = 0;
};

} // namespace pos
#endif // RAID1_H_
