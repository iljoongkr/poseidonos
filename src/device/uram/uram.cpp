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

#include "uram.h"

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <numa.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <utility>

#include "Air.h"
#include "src/array/device/array_device.h"
#include "src/bio/ubio.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/core_const.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/branch_prediction.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "uram_drv.h"
#include "uram_restore_completion.h"
#include "uram_device_context.h"

namespace pos
{
uint32_t UramBdev::reactorCount;
uint32_t UramBdev::ioatReactorCountNuma0;
uint32_t UramBdev::ioatReactorCountNuma1;

UramBdev::UramBdev(std::string name,
    uint64_t size,
    UramDrv* driverToUse,
    bool isPersistent)
: UBlockDevice(name, size, driverToUse),
  isPersistent(isPersistent),
  baseByteAddress(nullptr)
{
    property.type = DeviceType::NVRAM;
    property.mn = name;
    property.sn = name;
    // Uram dev only offload the ios to node 0
    reactorCount = AccelEngineApi::GetReactorCount();
    ioatReactorCountNuma0 = AccelEngineApi::GetIoatReactorCountPerNode(0);
    ioatReactorCountNuma1 = AccelEngineApi::GetIoatReactorCountPerNode(1);
}

UramBdev::~UramBdev(void)
{
}

bool
UramBdev::_RecoverBackup(DeviceContext* deviceContext)
{
    bool restoreSuccessful = true;

    string backupFileName = "/tmp/" + property.name +".uram.data";
    const uint32_t bytesPerHugepage = 2 * SZ_1MB;
    int fd = -1;

    try
    {
        fd = open(backupFileName.c_str(), O_RDONLY);
        if (0 > fd)
        {
            if (errno == ENOENT)
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::URAM_BACKUP_FILE_NOT_EXISTS;
                EventLevel eventLevel = EventLevel::INFO;
                throw std::make_pair(eventId, eventLevel);
            }
            else
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::URAM_BACKUP_FILE_OPEN_FAILED;
                EventLevel eventLevel = EventLevel::WARNING;
                throw std::make_pair(eventId, eventLevel);
            }
        }

        struct stat fileStat;
        int rc = fstat(fd, &fileStat);
        if (0 > rc)
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::URAM_BACKUP_FILE_STAT_FAILED;
            EventLevel eventLevel = EventLevel::WARNING;
            throw std::make_pair(eventId, eventLevel);
        }

        uint64_t pageCountBackup = fileStat.st_size / bytesPerHugepage;
        uint32_t unitsPerHugepage = bytesPerHugepage / Ubio::BYTES_PER_UNIT;

        for (uint64_t pageIndex = 0; pageIndex < pageCountBackup; pageIndex++)
        {
            UbioSmartPtr ubio(new Ubio(nullptr,
                DivideUp(bytesPerHugepage, Ubio::BYTES_PER_UNIT), 0));
            ubio->dir = UbioDir::Write;
            ubio->SetLba(pageIndex * unitsPerHugepage);
            ubio->SetUblock(shared_from_this());
            CallbackSmartPtr callback(new UramRestoreCompletion(ubio));
            ubio->SetCallback(callback);

            rc = read(fd, ubio->GetBuffer(), ubio->GetSize());
            if (bytesPerHugepage == rc)
            {
                UramRestoreCompletion::IncreasePendingUbio();
                SubmitAsyncIO(ubio);
            }
            else
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::URAM_BACKUP_FILE_READ_FAILED;
                std::string additionalMessage(
                    "Read page # " + std::to_string(pageIndex) + " failed");
                throw std::make_pair(eventId, additionalMessage);
            }
        }
    }
    catch (std::pair<POS_EVENT_ID, EventLevel> eventWithLevel)
    {
        POS_EVENT_ID eventId = eventWithLevel.first;
        EventLevel eventLevel = eventWithLevel.second;
        PosEventId::Print(eventId, eventLevel);
        if (eventId != POS_EVENT_ID::URAM_BACKUP_FILE_NOT_EXISTS)
        {
            restoreSuccessful = false;
        }
    }
    catch (std::pair<POS_EVENT_ID, std::string> eventWithMessage)
    {
        POS_EVENT_ID eventId = eventWithMessage.first;
        std::string& additionalMessage = eventWithMessage.second;
        PosEventId::Print(eventId, EventLevel::WARNING, additionalMessage);
        restoreSuccessful = false;
    }

    UramRestoreCompletion::WaitPendingUbioZero();

    if (fd >= 0)
    {
        close(fd);
        unlink(backupFileName.c_str());
    }
    return restoreSuccessful;
}

void*
UramBdev::GetByteAddress(void)
{
    return baseByteAddress;
}

void
UramBdev::_InitByteAddress(void)
{
    std::string backupDir = "/tmp/";
    std::string backupFilePostfix = ".uram.info";
    std::string uramName = this->GetName();
    std::string fileName = backupDir + uramName + backupFilePostfix;
    std::ifstream readFile;
    readFile.open(fileName);
    uint64_t byteAddressInt = 0, tmp1 = 0, tmp2 = 0;
    if (readFile.good())
    {
        readFile >> tmp1 >> byteAddressInt >> tmp2;
        readFile.close();
    }
    else
    {
        POS_TRACE_ERROR(POS_EVENT_ID::URAM_CONFIG_FILE_OPEN_FAILED,
            PosEventId::GetString(POS_EVENT_ID::URAM_CONFIG_FILE_OPEN_FAILED));
    }
    baseByteAddress = reinterpret_cast<void *>(byteAddressInt);
}
bool
UramBdev::_WrapupOpenDeviceSpecific(DeviceContext* deviceContext)
{
    // Reactor cannot handle Async operation for Uram in current implementation.
    // ioat poll cannot be called in Empty(), so, we restore the contents by IO worker.
    if (EventFrameworkApiSingleton::Instance()->IsReactorNow())
    {
        return true;
    }

    bool restoreSuccessful = false;
    if (isPersistent == true)
    {
        restoreSuccessful = true;
    }
    else
    {
        restoreSuccessful = _RecoverBackup(deviceContext);
    }
    _InitByteAddress();
    return restoreSuccessful;
}

DeviceContext*
UramBdev::_AllocateDeviceContext(void)
{
    DeviceContext* deviceContext = new UramDeviceContext(GetName());
    return deviceContext;
}

void
UramBdev::_ReleaseDeviceContext(DeviceContext* deviceContextToRelease)
{
    if (nullptr != deviceContextToRelease)
    {
        UramDeviceContext* deviceContext =
            static_cast<UramDeviceContext*>(deviceContextToRelease);
        delete deviceContext;
    }
}

int
UramBdev::SubmitAsyncIO(UbioSmartPtr ubio)
{
    UramDeviceContext* devCtx =
        static_cast<UramDeviceContext*>(_GetDeviceContext());
    if (devCtx->bdev_desc == nullptr)
    {
        int core = 0;
        if (ioatReactorCountNuma1 == 0) // If running machine is VM or Ioat is not support.
        {
            core = AccelEngineApi::GetReactorByIndex(requestCount % reactorCount);
        }
        else
        {
            core = AccelEngineApi::GetIoatReactorByIndex(requestCount % ioatReactorCountNuma1 + ioatReactorCountNuma0);
            if (unlikely(core < 0))
            {
                core = AccelEngineApi::GetIoatReactorByIndex(requestCount % reactorCount);
            }
        }

        UbioSmartPtr* ubioArgument = new UbioSmartPtr(ubio);
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(
            core, _RequestAsyncIo, ubioArgument);

        requestCount++;
        return 1;
    }

    return UBlockDevice::SubmitAsyncIO(ubio);
}

void
UramBdev::_RequestAsyncIo(void* arg1)
{
    UbioSmartPtr ubio = *static_cast<UbioSmartPtr*>(arg1);
    UBlockDevice* dev = ubio->GetUBlock();

    dev->SubmitAsyncIO(ubio);
    delete static_cast<UbioSmartPtr*>(arg1);
}

} // namespace pos
