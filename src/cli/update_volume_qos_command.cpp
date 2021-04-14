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

#include "src/cli/update_volume_qos_command.h"

#include "src/cli/cli_event_code.h"
#include "src/volume/volume_service.h"

namespace pos_cli
{
UpdateVolumeQosCommand::UpdateVolumeQosCommand(void)
{
}

UpdateVolumeQosCommand::~UpdateVolumeQosCommand(void)
{
}

string
UpdateVolumeQosCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("name"))
    {
        string volName = doc["param"]["name"].get<std::string>();
        uint64_t maxiops = 0;
        uint64_t maxbw = 0;

        if (doc["param"].contains("maxiops") &&
            doc["param"]["maxiops"].is_number_unsigned() == true)
        {
            maxiops = doc["param"]["maxiops"].get<uint64_t>();
        }
        if (doc["param"].contains("maxbw") &&
            doc["param"]["maxbw"].is_number_unsigned() == true)
        {
            maxbw = doc["param"]["maxbw"].get<uint64_t>();
        }

        IVolumeManager* volMgr =
            VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

        int ret = FAIL;
        if (volMgr != nullptr)
        {
            ret = volMgr->UpdateQoS(volName, maxiops, maxbw);
        }

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("UPDATEVOLUMEQOS", rid, ret,
                "QoS of " + volName + " updated successfully", GetPosInfo());
        }
        else
        {
            return jFormat.MakeResponse("UPDATEVOLUMEQOS", rid, ret,
                "failed to update QoS of " + volName + "(code:" + to_string(ret) + ")",
                GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "UPDATEVOLUMEQOS", rid, BADREQUEST,
            "volume name is not entered", GetPosInfo());
    }
}
}; // namespace pos_cli
