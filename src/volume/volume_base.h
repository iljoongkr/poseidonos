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

#ifndef VOLUME_BASE_H_
#define VOLUME_BASE_H_

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

#define MAX_VOLUME_COUNT (256)

namespace pos
{
enum VolumeStatus
{
    Unmounted,
    Mounted,
    MaxVolumeStatus
};

const uint32_t KIOPS = 1000;
const uint32_t MIB_IN_BYTE = 1024 * 1024;
const uint64_t MIN_IOPS_LIMIT = 10;
const uint64_t MIN_BW_LIMIT = 10;
const uint64_t MAX_IOPS_LIMIT = UINT64_MAX / KIOPS;
const uint64_t MAX_BW_LIMIT = UINT64_MAX / MIB_IN_BYTE;

class VolumeBase
{
public:
    VolumeBase(std::string arrayName, int arrayIdx, std::string volName, uint64_t volSizeByte);
    VolumeBase(std::string arrayName, int arrayIdx, std::string volName, std::string uuid, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw);
    virtual ~VolumeBase(void);
    int Mount(void);
    int Unmount(void);
    void LockStatus(void);
    void UnlockStatus(void);
    uint64_t TotalSize(void);
    uint64_t UsedSize(void);
    uint64_t RemainingSize(void);

    std::string
    GetName(void)
    {
        return name;
    }
    std::string
    GetUuid(void)
    {
        return uuid;
    }
    std::string
    GetArrayName(void)
    {
        return array;
    }
    int
    GetArray(void)
    {
        return arrayId;
    }
    std::string
    GetSubnqn(void)
    {
        return subNqn;
    }
    void SetSubnqn(std::string inputSubNqn);
    void SetUuid(std::string inputUuid);
    VolumeStatus
    GetStatus(void)
    {
        return status;
    }
    bool
    IsValid(void)
    {
        return isValid;
    }
    void
    SetValid(bool valid)
    {
        isValid = valid;
    }
    uint64_t
    MaxIOPS(void)
    {
        return maxiops;
    }
    uint64_t
    MaxBW(void)
    {
        return maxbw;
    }
    int SetMaxIOPS(uint64_t val);
    int SetMaxBW(uint64_t val);
    void
    Rename(std::string val)
    {
        name = val;
    }
    void
    Resize(uint64_t val)
    {
        totalSize = val;
    }

    int ID;

protected:
    VolumeStatus status;
    std::string name;
    std::string uuid;
    std::string array;
    int arrayId;
    std::string subNqn = "";
    uint64_t maxiops = 0; // 0 == unlimited
    uint64_t maxbw = 0;   // 0 == unlimited

    bool isValid = true;
    uint64_t totalSize;
    std::mutex statusMutex;
    static const int INVALID_VOL_ID = -1;
};

} // namespace pos

#endif // VOLUME_BASE_H_
