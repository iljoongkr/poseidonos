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
#include "src/telemetry/telemetry_client/grpc_global_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include <list>
#include <map>
#include <string>
#include <vector>

namespace pos
{
class GrpcGlobalPublisher;

class TelemetryClient
{
public:
    TelemetryClient(TelemetryManagerService* telemetryManager_, std::shared_ptr<grpc::Channel> channel_);
    TelemetryClient(void);
    virtual ~TelemetryClient(void);
    virtual int RegisterPublisher(std::string name, TelemetryPublisher* client);
    virtual int DeregisterPublisher(std::string name);
    virtual bool StartPublisher(std::string name);
    virtual bool StopPublisher(std::string name);
    virtual bool IsPublisherRunning(std::string name);
    virtual bool StartAllPublisher(void);
    virtual bool StopAllPublisher(void);

    virtual int CollectValue(std::string name, std::string id, MetricUint32& outLog);
    virtual list<MetricUint32> CollectList(std::string name);

private:
    std::map<std::string, TelemetryPublisher*> publisherList;
    GrpcGlobalPublisher* globalPublisher;
};

using TelemetryClientSingleton = Singleton<TelemetryClient>;

} // namespace pos
