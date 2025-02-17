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


#include "src/include/array_mgmt_policy.h"
#include "src/lib/singleton.h"
#include "src/sys_event/volume_event.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace pos
{
class VolumeEventPublisher
{
public:
    VolumeEventPublisher(void);
    virtual ~VolumeEventPublisher(void);
    void RegisterSubscriber(VolumeEvent* subscriber, std::string arrayName, int arrayId);
    void RemoveSubscriber(VolumeEvent* subscriber, std::string arrayName, int arrayId);
    bool NotifyVolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo);
    bool NotifyVolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo);
    bool NotifyVolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo);
    bool NotifyVolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo);
    bool NotifyVolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo);
    bool NotifyVolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo);
    void NotifyVolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo);

private:
    vector<std::pair<int, VolumeEvent*>> subscribers;
};

using VolumeEventPublisherSingleton = Singleton<VolumeEventPublisher>;
} // namespace pos
