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

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <list>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "proto/generated/cpp/telemetry.grpc.pb.h"
#include "proto/generated/cpp/telemetry.pb.h"
#include "src/helper/json_helper.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

using namespace ::grpc;

namespace pos
{
class ProtoBufCreateChannel : public TelemetryManager::Service
{
public:
    explicit ProtoBufCreateChannel(string serverAddress);
    virtual ~ProtoBufCreateChannel(void);

    /* for grpc Test */
    ::grpc::Status PublishToServer(void);
    ::grpc::Status ConfigureToServer(void);
    ::grpc::Status CollectToServer(void);
    ::grpc::Status EnableToServer(void);
    ::grpc::Status DisableToServer(void);

private:
    std::unique_ptr<TelemetryManager::Stub> stub_;
};

} // namespace pos
