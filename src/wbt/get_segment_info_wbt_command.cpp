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


#include "src/allocator/i_allocator_wbt.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/wbt/get_segment_info_wbt_command.h"

#include <string>
namespace pos
{
GetSegmentInfoWbtCommand::GetSegmentInfoWbtCommand(void)
:   WbtCommand(GET_SEGMENT_INFO, "get_segment_info")
{
}

GetSegmentInfoWbtCommand::~GetSegmentInfoWbtCommand(void)
{
}

int
GetSegmentInfoWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    int res = -1;
    std::string arrayName = _GetParameter(argv, "array");
    if (0 == arrayName.length())
    {
        return res;
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        return res;
    }

    IAllocatorWbt* iAllocatorWbt = AllocatorServiceSingleton::Instance()->GetIAllocatorWbt(arrayName);
    res = iAllocatorWbt->GetMeta(WBT_SEGMENT_VALID_COUNT, argv["output"].get<std::string>());

    return res;
}

} // namespace pos
