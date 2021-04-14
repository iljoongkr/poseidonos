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

#include "src/cli/device_monitoring_state_command.h"

#include <utility>
#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/device/device_manager.h"

namespace pos_cli
{
DeviceMonitoringStateCommand::DeviceMonitoringStateCommand(void)
{
}

DeviceMonitoringStateCommand::~DeviceMonitoringStateCommand(void)
{
}

string DeviceMonitoringStateCommand::Execute(json& doc, string rid)
{
    vector<pair<string, string>> monitors;
    monitors = pos::DeviceManagerSingleton::Instance()->MonitoringState();
    JsonFormat jFormat;

    if (monitors.size() == 0)
    {
        return jFormat.MakeResponse(
            "DEVICEMONITORINGSTATE", rid, SUCCESS, "no any device monitor exists",
            GetPosInfo());
    }

    JsonArray array("monitorlist");
    for (size_t i = 0; i < monitors.size(); i++)
    {
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("name", "\"" + monitors[i].first + "\""));
        elem.SetAttribute(
            JsonAttribute("state", "\"" + monitors[i].second + "\""));
        array.AddElement(elem);
    }
    JsonElement data("data");
    data.SetArray(array);
    return jFormat.MakeResponse("DEVICEMONITORINGSTATE", rid, SUCCESS, "DONE",
        data, GetPosInfo());
}
}; // namespace pos_cli
