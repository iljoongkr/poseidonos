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

#include "src/cli/create_subsystem_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/spdk_rpc_client.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/network/nvmf_target.h"

namespace pos_cli
{
CreateSubsystemCommand::CreateSubsystemCommand(void)
{
}

CreateSubsystemCommand::~CreateSubsystemCommand(void)
{
}

string
CreateSubsystemCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;

    int ret = 0;
    ret = _CreateSubsystem(doc);
    if (ret != SUCCESS)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::IONVMF_FAIL_TO_CREATE_SUBSYSTEM;
        return jFormat.MakeResponse(
            "CREATESUBSYSTEM", rid, static_cast<int>(eventId),
            PosEventId::GetString(eventId), GetPosInfo());
    }

    return jFormat.MakeResponse(
        "CREATESUBSYSTEM", rid, SUCCESS,
        "Subsystem ( " + doc["param"]["name"].get<string>() + " ) has been created.", GetPosInfo());
}

int
CreateSubsystemCommand::_CreateSubsystem(json& doc)
{
    SpdkRpcClient rpcClient;
    NvmfTarget target;
    subnqn = doc["param"]["name"].get<string>();

    if ("CREATESUBSYSTEMAUTO" == doc["command"].get<string>())
    {
        if (nullptr != target.FindSubsystem(subnqn))
        {
            return SUCCESS;
        }
        _SetDefaultOptions(doc);
    }
    else if ("CREATESUBSYSTEM" == doc["command"].get<string>())
    {
        if (nullptr != target.FindSubsystem(subnqn))
        {
            errorMessage = "Failed to create subsystem. Suggested subnqn name already exists. ";
            return FAIL;
        }
        if (doc["param"].contains("sn"))
        {
            serialNumber = doc["param"]["sn"].get<string>();
        }
        if (doc["param"].contains("mn"))
        {
            modelNumber = doc["param"]["mn"].get<string>();
        }
        if (doc["param"].contains("max_namespaces"))
        {
            maxNamespaces = doc["param"]["max_namespaces"].get<uint32_t>();
        }
        if (doc["param"].contains("allow_any_host"))
        {
            allowAnyHost = doc["param"]["allow_any_host"].get<bool>();
        }
        if (doc["param"].contains("ana_reporting"))
        {
            anaReporting = doc["param"]["ana_reporting"].get<bool>();
        }
    }

    auto ret = rpcClient.SubsystemCreate(
        subnqn,
        serialNumber,
        modelNumber,
        maxNamespaces,
        allowAnyHost,
        anaReporting);
    if (ret.first != SUCCESS)
    {
        errorMessage = "Failed to create subsystem. " + ret.second;
    }
    return ret.first;
}

void
CreateSubsystemCommand::_SetDefaultOptions(json& doc)
{
    string key("subsystem");
    string number;

    size_t found = subnqn.rfind(key);
    if (found != string::npos)
    {
        size_t index = found + key.length();
        number = subnqn.substr(index);
        serialNumber += number;
    }
    allowAnyHost = true;
}
} // namespace pos_cli
