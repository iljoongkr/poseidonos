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

#include "parity_location_wbt_command.h"

#include <cstdlib>
#include <fstream>
#include <string>

#include "src/array/array.h"
#include "src/array/ft/raid5.h"
#include "src/array/partition/stripe_partition.h"
#include "src/array_mgmt/array_manager.h"
#include "src/device/device_manager.h"

namespace pos
{
ParityLocationWbtCommand::ParityLocationWbtCommand(void)
:   WbtCommand(PARITY_LOCATION, "parity_location")
{
}

ParityLocationWbtCommand::~ParityLocationWbtCommand(void)
{
}

int
ParityLocationWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    std::string coutfile = "output.txt";
    std::ofstream out(coutfile.c_str(), std::ofstream::app);

    if (!argv.contains("dev") || !argv.contains("lba") || !argv.contains("name"))
    {
        out << "invalid parameter" << endl;
        out.close();
        return 0;
    }

    string devName = argv["dev"].get<std::string>();
    uint64_t lba = atoi(argv["lba"].get<std::string>().c_str());
    string arrayName = argv["name"].get<std::string>();


    ArrayComponents* compo = ArrayManagerSingleton::Instance()->_FindArray(arrayName);
    if (compo == nullptr)
    {
        out << "array does not exist" << endl;
        out.close();
        return 0;
    }
    Array* sysArray = compo->GetArray();
    if (sysArray == nullptr)
    {
        out << "array does not exist" << endl;
        out.close();
        return 0;
    }
    if (sysArray->state->IsMounted() == false)
    {
        out << "array is not mounted" << endl;
        out.close();
        return 0;
    }

    ArrayDeviceType devType;
    ArrayDevice* arrayDev = nullptr;
    DevName name(devName);
    UblockSharedPtr uBlock = DeviceManagerSingleton::Instance()->GetDev(name);
    tie(arrayDev, devType) = sysArray->devMgr_->GetDev(uBlock);
    if (arrayDev == nullptr || devType != ArrayDeviceType::DATA)
    {
        out << "device not found" << endl;
        return 0;
    }
    PhysicalBlkAddr pba = {
        .lba = lba,
        .arrayDev = arrayDev};
    StripePartition* ptn = static_cast<StripePartition*>(
        sysArray->ptnMgr->partitions_[PartitionType::USER_DATA]);
    Raid5* method = static_cast<Raid5*>(ptn->GetMethod());

    FtBlkAddr fba = ptn->_P2FTranslate(pba);
    uint32_t parityIndex = method->_GetParityOffset(fba.stripeId);
    ArrayDevice* parityDev = ptn->devs_.at(parityIndex);

    out << "device name : " << parityDev->GetUblock()->GetName() << endl;
    out << "lba : " << lba << endl;

    out << std::endl;
    out.close();
    return 0;
}

} // namespace pos
