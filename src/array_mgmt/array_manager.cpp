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

#include "array_manager.h"

#include <list>

#include "src/device/device_manager.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/mbr/abr_manager.h"

namespace pos
{
ArrayManager::ArrayManager()
: ArrayManager(new ArrayRebuilder(this), new AbrManager(), DeviceManagerSingleton::Instance(),
    TelemetryClientSingleton::Instance(),
    [](string name, IArrayRebuilder* arrayRebuilder, IAbrControl* iAbrControl)
    {
        return new ArrayComponents(name, arrayRebuilder, iAbrControl);
    })
{
    // delegated to other constructor
}

ArrayManager::ArrayManager(ArrayRebuilder* arrayRebuilder, AbrManager* abrManager,
    DeviceManager* deviceManager, TelemetryClient* telClient,
    function<ArrayComponents*(string, IArrayRebuilder*, IAbrControl*)> arrayComponentsFactory)
: arrayRebuilder(arrayRebuilder),
  abrManager(abrManager),
  deviceManager(deviceManager),
  telClient(telClient),
  arrayComponentsFactory(arrayComponentsFactory)
{
    if (deviceManager != nullptr)
    {
        deviceManager->SetDeviceEventCallback(this);
    }
    else
    {
        POS_TRACE_DEBUG(EID(ARRAY_MANAGER_DEBUG_MSG), "Ignoring DeviceManager's subscription to DeviceEvent. It's not okay if it's for prod code (not unit test)");
    }
}

ArrayManager::~ArrayManager()
{
    if (arrayRebuilder != nullptr)
    {
        POS_TRACE_INFO(EID(ARRAY_MANAGER_DEBUG_MSG), "Deleting ArrayRebuilder");
        delete arrayRebuilder;
    }

    if (abrManager != nullptr)
    {
        POS_TRACE_INFO(EID(ARRAY_MANAGER_DEBUG_MSG), "Deleting AbrManager");
        delete abrManager;
    }

    for (auto iter : arrayList)
    {
        ArrayComponents* array = _FindArray(iter.first);
        if (array != nullptr)
        {
            POS_TRACE_INFO(EID(ARRAY_MANAGER_DEBUG_MSG), "Deleting ArrayComponents for {}", iter.first);
            delete array;
        }
        else
        {
            POS_TRACE_WARN(EID(ARRAY_MANAGER_DEBUG_MSG), "Couldn't find ArrayComponents for {}", iter.first);
        }
    }
}

int
ArrayManager::Create(string name, DeviceSet<string> devs, string raidtype)
{
    if (_FindArray(name) != nullptr)
    {
        return (int)POS_EVENT_ID::ARRAY_ALREADY_EXIST;
    }

    if (arrayList.size() >= ArrayMgmtPolicy::MAX_ARRAY_CNT)
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_CNT_EXCEEDED,
            "ArrayManager cnt exceeded. current: {}", arrayList.size());
        return (int)POS_EVENT_ID::ARRAY_CNT_EXCEEDED;
    }

    ArrayComponents* array = arrayComponentsFactory(name, arrayRebuilder, abrManager);
    int ret = array->Create(devs, raidtype);
    if (ret == (int)POS_EVENT_ID::SUCCESS)
    {
        arrayList.emplace(name, array);
    }
    else
    {
        POS_TRACE_WARN(ret, "ArrayManager is cleaning up ArrayComponents of {} because of array creation failure", name);
        delete array;
    }
    return ret;
}

int
ArrayManager::Delete(string name)
{
    ArrayComponents* array = _FindArray(name);
    if (array == nullptr)
    {
        if (AbrExists(name))
        {
            int result = _DeleteFaultArray(name);
            return result;
        }
        return (int)POS_EVENT_ID::ARRAY_WRONG_NAME;
    }

    int ret = array->Delete();
    if (ret == (int)POS_EVENT_ID::SUCCESS)
    {
        delete array;
        arrayList.erase(name);
    }

    return ret;
}

int
ArrayManager::Mount(string name)
{
    return _ExecuteOrHandleErrors([&](ArrayComponents* array)
    {
        telClient->RegisterPublisher(name, array->GetTelemetryPublisher());
        int ret = array->Mount();
        if (ret !=  EID(SUCCESS))
        {
            telClient->DeregisterPublisher(name);
        }
        return ret;
    }, name);
}

int
ArrayManager::Unmount(string name)
{
    return _ExecuteOrHandleErrors([&](ArrayComponents* array)
    {
        int ret = array->Unmount();
        if (ret == EID(SUCCESS))
        {
            telClient->DeregisterPublisher(name);
        }
        return ret;
    }, name);
}

int
ArrayManager::AddDevice(string name, string dev)
{
    return _ExecuteOrHandleErrors([&dev](ArrayComponents* array)
    {
        return array->GetArray()->AddSpare(dev);
    }, name);
}

int
ArrayManager::RemoveDevice(string name, string dev)
{
    return _ExecuteOrHandleErrors([&dev](ArrayComponents* array)
    {
        return array->GetArray()->RemoveSpare(dev);
    }, name);
}

int
ArrayManager::_ExecuteOrHandleErrors(std::function<int(ArrayComponents*)> f, string name)
{
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        return f(array);
    }
    else if (AbrExists(name))
    {
        return (int)POS_EVENT_ID::ARRAY_LOAD_FAIL;
    }
    else
    {
        return (int)POS_EVENT_ID::ARRAY_WRONG_NAME;
    }
}

int
ArrayManager::DeviceDetached(UblockSharedPtr dev)
{
    ArrayComponents* array = _FindArrayWithDevSN(dev->GetSN());
    if (array != nullptr)
    {
        return array->GetArray()->DetachDevice(dev);
    }
    else
    {
        POS_TRACE_WARN(EID(ARRAY_NOT_FOUND), "No array found for device serial number {}. DeviceDetached event will be ignored.", dev->GetSN());
    }
    return 0; // this function will be void type when device lock is removed
}

void
ArrayManager::DeviceAttached(UblockSharedPtr dev)
{
    // do nothing. reserved for future.
}

int
ArrayManager::GetAbrList(std::vector<ArrayBootRecord>& abrList)
{
    int result = abrManager->GetAbrList(abrList);
    return result;
}

ComponentsInfo*
ArrayManager::GetInfo(string name)
{
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        return array->GetInfo();
    }
    else
    {
        POS_TRACE_WARN(EID(ARRAY_COMPONENTS_DEBUG_MSG), "No ArrayComponents found for {}. Returning null IArrayInfo.", name);
        return nullptr;
    }
}

ComponentsInfo*
ArrayManager::GetInfo(uint32_t arrayIdx)
{
    for (auto iter = arrayList.begin(); iter != arrayList.end(); iter++)
    {
        if (iter->second->GetArray()->GetIndex() == arrayIdx)
        {
            return iter->second->GetInfo();
        }
    }

    return nullptr;
}

int
ArrayManager::PrepareRebuild(string name, bool& resume)
{
    ArrayComponents* array = _FindArray(name);
    if (array == nullptr)
    {
        return (int)POS_EVENT_ID::ARRAY_WRONG_NAME;
    }

    return array->PrepareRebuild(resume);
}

void
ArrayManager::RebuildDone(string name)
{
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        array->RebuildDone();
    }
    else
    {
        POS_TRACE_WARN(EID(ARRAY_NOT_FOUND), "Cannot invoke RebuildDone because the array {} isn't found", name);
    }
}

int
ArrayManager::Load(list<string>& failedArrayList)
{
    std::vector<ArrayBootRecord> abrList;
    std::vector<ArrayBootRecord>::iterator it;
    int result = abrManager->GetAbrList(abrList);
    int loadResult = 0;
    if (result == 0)
    {
        for (it = abrList.begin(); it != abrList.end(); it++)
        {
            loadResult = _Load(it->arrayName);
            if (loadResult != 0)
            {
                string arrayName(it->arrayName);
                POS_TRACE_ERROR(loadResult, "Array " + arrayName + " load failed");
                failedArrayList.push_back(arrayName);
                result = loadResult;
            }
        }
    }
    else
    {
        POS_TRACE_ERROR(result, "Failed to get abr list");
    }

    return result;
}

int
ArrayManager::_Load(string name)
{
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        return (int)POS_EVENT_ID::ARRAY_ALREADY_EXIST;
    }

    array = arrayComponentsFactory(name, arrayRebuilder, abrManager);
    int ret = array->Load();
    if (ret == (int)POS_EVENT_ID::SUCCESS)
    {
        arrayList.emplace(name, array);
    }
    else
    {
        delete array;
    }

    return ret;
}

int
ArrayManager::ResetMbr(void)
{
    int result = 0;
    int deleteResult = 0;
    for (auto iter : arrayList)
    {
        ArrayComponents* array = _FindArray(iter.first);
        if (array != nullptr)
        {
            result = array->GetArray()->CheckDeletable();
            if (result != 0)
            {
                POS_TRACE_WARN(deleteResult, "Cannot delete array " + iter.first);
                return result;
            }
        }
        else
        {
            result = (int)POS_EVENT_ID::ARRAY_STATE_NOT_EXIST;
            POS_TRACE_ERROR(result, "Cannot find array " + iter.first);
            return result;
        }
    }

    result = 0;
    for (auto iter = arrayList.begin(); iter != arrayList.end();)
    {
        deleteResult = iter->second->Delete();
        if (deleteResult == 0)
        {
            delete iter->second;
            iter = arrayList.erase(iter);
        }
        else
        {
            iter++;
            POS_TRACE_ERROR(deleteResult, "Failed to delete array : {}", iter->first);
            result = deleteResult;
        }
    }

    if (result == 0)
    {
        result = abrManager->ResetMbr();
    }

    return result;
}

ArrayComponents*
ArrayManager::_FindArray(string name)
{
    auto it = arrayList.find(name);
    if (it == arrayList.end())
    {
        return nullptr;
    }

    return it->second;
}

ArrayComponents*
ArrayManager::_FindArrayWithDevSN(string devSN)
{
    string arrayName = abrManager->FindArrayWithDeviceSN(devSN);
    if (arrayName == "")
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_NOT_FOUND;
        POS_TRACE_INFO(eventId, "There is no array that owns device '{}'", devSN);
        return nullptr;
    }
    ArrayComponents* array = _FindArray(arrayName);

    return array;
}

bool
ArrayManager::AbrExists(string arrayName)
{
    std::vector<ArrayBootRecord> abrList;
    int result = GetAbrList(abrList);

    if (result == 0)
    {
        for (auto const &abr : abrList)
        {
            if (abr.arrayName == arrayName)
            {
                return true;
            }
        }
    }

    return false;
}

int
ArrayManager::_DeleteFaultArray(string arrayName)
{
    ArrayMeta meta;
    unsigned int arrayIndex = -1;
    int result = abrManager->LoadAbr(arrayName, meta, arrayIndex);
    if (result != 0)
    {
        return result;
    }
    result = abrManager->DeleteAbr(arrayName, meta);

    return result;
}

void
ArrayManager::SetArrayComponentMap(const map<string, ArrayComponents*>& arrayCompMap)
{
    // The member variable says it's array"List", but it's actually a map.
    this->arrayList = arrayCompMap;
}

const map<string, ArrayComponents*>&
ArrayManager::GetArrayComponentMap(void)
{
    return arrayList;
}

} // namespace pos
