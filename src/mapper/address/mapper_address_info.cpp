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

#include "src/array_mgmt/array_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/include/address_type.h"
#include "src/mapper/address/mapper_address_info.h"

#include <string>

namespace pos
{
MapperAddressInfo::MapperAddressInfo(IArrayInfo* iArrayInfo_)
: iArrayInfo(iArrayInfo_)
{
    maxVsid = 0;
    blksPerStripe = 0;
    numWbStripes = 0;
    mpageSize = 0;
}

void
MapperAddressInfo::SetupAddressInfo(int mpageSize_)
{
    const PartitionLogicalSize* udSize = iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    const PartitionLogicalSize* wbSize = iArrayInfo->GetSizeInfo(PartitionType::WRITE_BUFFER);

    maxVsid = udSize->totalStripes;
    blksPerStripe = udSize->blksPerStripe;
    numWbStripes = wbSize->totalStripes;
    mpageSize = mpageSize_;
}

uint64_t
MapperAddressInfo::GetMpageSize(void)
{
    return mpageSize;
}

std::string
MapperAddressInfo::GetArrayName(void)
{
    return iArrayInfo->GetName();
}

int
MapperAddressInfo::GetArrayId(void)
{
    return iArrayInfo->GetIndex();
}

} // namespace pos
