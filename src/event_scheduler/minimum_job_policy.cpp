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

#include "src/event_scheduler/minimum_job_policy.h"
#include "src/event_scheduler/event_scheduler.h"
namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constuctor
 *
 * @Param    workerCountInput
 */
/* --------------------------------------------------------------------------*/
MinimumJobPolicy::MinimumJobPolicy(unsigned int workerCountInput)
: workerCount(workerCountInput),
  currentWorkerID(workerCount - 1)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Destructor
 */
/* --------------------------------------------------------------------------*/
MinimumJobPolicy::~MinimumJobPolicy(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Return EventWorkerID for next execution
 *
 *
 * @Returns  EventWorkerID for next execution
 */
/* --------------------------------------------------------------------------*/
unsigned int
MinimumJobPolicy::GetProperWorkerID(uint32_t numa)
{
    uint32_t workerID = EventSchedulerSingleton::Instance()->GetWorkerIDMinimumJobs(numa);
    return workerID;
}
} // namespace pos
