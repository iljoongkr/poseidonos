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

#include "abr_manager.h"

#include <list>

#include "src/array/device/array_device_list.h"
#include "src/helper/time_helper.h"
#include "src/include/array_device_state.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/mbr/mbr_manager.h"
#include "src/mbr/mbr_util.h"

namespace pos
{
AbrManager::AbrManager(void)
: AbrManager(new MbrManager())
{
}

AbrManager::AbrManager(MbrManager* mbrMgr)
: mbrManager(mbrMgr)
{
}

AbrManager::~AbrManager(void)
{
    delete mbrManager;
}

int
AbrManager::LoadAbr(string arrayName, ArrayMeta& meta, unsigned int& arrayIndex)
{
    struct ArrayBootRecord* abr = nullptr;
    mbrManager->GetAbr(arrayName, &abr, arrayIndex);
    if (abr == nullptr)
    {
        int result = (int)POS_EVENT_ID::MBR_ABR_NOT_FOUND;
        POS_TRACE_WARN(result, "No array found with arrayName :{}", arrayName);
        return result;
    }

    meta.arrayName = abr->arrayName;
    int nvmNum = abr->nvmDevNum;
    int dataNum = abr->dataDevNum;
    int spareNum = abr->spareDevNum;

    int devIndex = 0;

    for (int i = 0; i < nvmNum; i++)
    {
        devIndex = i;
        string uid(abr->devInfo[devIndex].deviceUid);
        meta.devs.nvm.push_back(DeviceMeta(uid));
    }

    for (int i = 0; i < dataNum; i++)
    {
        devIndex = nvmNum + i;
        string uid(abr->devInfo[devIndex].deviceUid);
        int state = abr->devInfo[devIndex].deviceState;
        DeviceMeta deviceMeta(uid, static_cast<ArrayDeviceState>(state));
        meta.devs.data.push_back(deviceMeta);
    }

    for (int i = 0; i < spareNum; i++)
    {
        devIndex = nvmNum + dataNum + i;
        string uid(abr->devInfo[devIndex].deviceUid);
        meta.devs.spares.push_back(DeviceMeta(uid));
    }

    meta.createDatetime = abr->createDatetime;
    meta.updateDatetime = abr->updateDatetime;
    return 0;
}

int
AbrManager::SaveAbr(string arrayName, ArrayMeta& meta)
{
    struct ArrayBootRecord* abr = nullptr;
    unsigned int arrayIndex;
    mbrManager->GetAbr(arrayName, &abr, arrayIndex);
    if (abr == nullptr)
    {
        int ret = (int)POS_EVENT_ID::MBR_ABR_NOT_FOUND;
        POS_TRACE_ERROR(ret, "Cannot save abr, abr not found");
        return ret;
    }

    int nvmNum = meta.devs.nvm.size();
    int dataNum = meta.devs.data.size();
    int spareNum = meta.devs.spares.size();

    abr->nvmDevNum = nvmNum;
    abr->dataDevNum = dataNum;
    abr->spareDevNum = spareNum;
    abr->totalDevNum = nvmNum + dataNum + spareNum;

    CopyData(abr->arrayName, meta.arrayName, ARRAY_NAME_SIZE);
    CopyData(abr->metaRaidType, meta.metaRaidType, META_RAID_TYPE_SIZE);
    CopyData(abr->dataRaidType, meta.dataRaidType, DATA_RAID_TYPE_SIZE);
    CopyData(abr->updateDatetime, GetCurrentTimeStr("%Y-%m-%d %X %z", DATE_SIZE), DATE_SIZE);

    for (int i = 0; i < nvmNum; i++)
    {
        int deviceIndex = i;
        abr->devInfo[deviceIndex].deviceType =
            (int)ArrayDeviceType::NVM;
        CopyData(abr->devInfo[deviceIndex].deviceUid,
            meta.devs.nvm.at(i).uid, DEVICE_UID_SIZE);
    }

    for (int i = 0; i < dataNum; i++)
    {
        int deviceIndex = nvmNum + i;
        abr->devInfo[deviceIndex].deviceType =
            (int)ArrayDeviceType::DATA;
        CopyData(abr->devInfo[deviceIndex].deviceUid,
            meta.devs.data.at(i).uid, DEVICE_UID_SIZE);
        abr->devInfo[deviceIndex].deviceState =
            (int)meta.devs.data.at(i).state;
    }

    for (int i = 0; i < spareNum; i++)
    {
        int deviceIndex = nvmNum + dataNum + i;
        abr->devInfo[deviceIndex].deviceType =
            (int)ArrayDeviceType::SPARE;
        CopyData(abr->devInfo[deviceIndex].deviceUid,
            meta.devs.spares.at(i).uid, DEVICE_UID_SIZE);
    }

    abr->mfsInit = GetMfsInit(arrayName);

    meta.createDatetime = abr->createDatetime;
    meta.updateDatetime = abr->updateDatetime;
    mbrManager->UpdateDeviceIndexMap(arrayName);

    int result = mbrManager->SaveMbr();
    return result;
}

bool
AbrManager::GetMfsInit(string arrayName)
{
    struct ArrayBootRecord* abr = nullptr;
    unsigned int arrayIndex;
    mbrManager->GetAbr(arrayName, &abr, arrayIndex);
    int value = abr->mfsInit;
    return value ? true : false;
}

int
AbrManager::SetMfsInit(string arrayName, bool value)
{
    struct ArrayBootRecord* abr = nullptr;
    unsigned int arrayIndex;
    mbrManager->GetAbr(arrayName, &abr, arrayIndex);
    int mfsInit = value == true ? 1 : 0;
    abr->mfsInit = mfsInit;
    mbrManager->SaveMbr();
    return 0;
}

int
AbrManager::CreateAbr(string arrayName, ArrayMeta& meta, unsigned int& arrayIndex)
{
    return mbrManager->CreateAbr(arrayName, meta, arrayIndex);
}

int
AbrManager::DeleteAbr(string arrayName, ArrayMeta& meta)
{
    return mbrManager->DeleteAbr(arrayName, meta);
}

int
AbrManager::ResetMbr(void)
{
    return mbrManager->ResetMbr();
}

int
AbrManager::GetAbrList(std::vector<ArrayBootRecord>& abrList)
{
    int result = mbrManager->LoadMbr();
    if (result != 0)
    {
        return result;
    }
    
    result = mbrManager->GetAbrList(abrList);
    return result;
}

string
AbrManager::FindArrayWithDeviceSN(string devSN)
{
    string arrayName = mbrManager->FindArrayWithDeviceSN(devSN);

    return arrayName;
}

} // namespace pos
