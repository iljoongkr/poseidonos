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

#include <map>

#include "src/qos/internal_manager.h"
#include "src/qos/monitoring_manager.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_context.h"

namespace pos
{
class QosContext;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 */
/* --------------------------------------------------------------------------*/
class QosMonitoringManagerArray
{
public:
    QosMonitoringManagerArray(uint32_t arrayIndex, QosContext* qosCtx);
    ~QosMonitoringManagerArray(void);
    void UpdateContextUserVolumePolicy(void);
    void UpdateVolumeParameter(uint32_t volId);
    void UpdateContextResourceDetails(void);
    void UpdateContextUserRebuildPolicy(void);
    bool VolParamActivities(uint32_t volId, uint32_t rector);

private:
    uint32_t arrayId;
    QosContext* qosContext;
    void _UpdateContextActiveReactorVolumes(uint32_t reactor, uint32_t volId);
    void _UpdateContextActiveVolumes(uint32_t volumeId);
    void _UpdateContextVolumeThrottle(uint32_t volId);
    void _UpdateContextVolumeParameter(uint32_t volId);
    bw_iops_parameter volParams[MAX_VOLUME_COUNT];
    void _UpdateVolumeReactorParameter(uint32_t volId, uint32_t reactor);
};
} // namespace pos
