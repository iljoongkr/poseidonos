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

#include "meta_volume_manager.h"
#include <string>
#include <vector>

namespace pos
{
MetaVolumeManager::MetaVolumeManager(MetaVolumeHandler* _volHandler,
        MetaVolumeContainer* _volContainer)
: volumeSpcfReqHandler{},
  globalRequestHandler{},
  metaStorage(nullptr)
{
    volHandler = _volHandler;
    volContainer = _volContainer;

    if (nullptr == volHandler)
        volHandler = new MetaVolumeHandler();

    if (nullptr == volContainer)
        volContainer = new MetaVolumeContainer();

    _InitRequestHandler();
}

MetaVolumeManager::~MetaVolumeManager(void)
{
    if (nullptr != volHandler)
        delete volHandler;

    if (nullptr != volContainer)
        delete volContainer;
}

void
MetaVolumeManager::_InitRequestHandler(void)
{
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileOpen,
                    &MetaVolumeHandler::HandleOpenFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileClose,
                    &MetaVolumeHandler::HandleCloseFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileCreate,
                    &MetaVolumeHandler::HandleCreateFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileDelete,
                    &MetaVolumeHandler::HandleDeleteFileReq);

    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::CheckFileExist,
                    &MetaVolumeHandler::HandleCheckFileExist);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetDataChunkSize,
                    &MetaVolumeHandler::HandleGetDataChunkSizeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::CheckFileAccessible,
                    &MetaVolumeHandler::HandleCheckFileAccessibleReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetFileSize,
                    &MetaVolumeHandler::HandleGetFileSizeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetTargetMediaType,
                    &MetaVolumeHandler::HandleGetTargetMediaTypeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetFileBaseLpn,
                    &MetaVolumeHandler::HandleGetFileBaseLpnReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetAvailableSpace,
                    &MetaVolumeHandler::HandleGetFreeFileRegionSizeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetMaxMetaLpn,
                    &MetaVolumeHandler::HandleGetMaxMetaLpnReq);

    _RegisterGlobalMetaReqHandler(MetaFsFileControlType::EstimateDataChunkSize,
                    &MetaVolumeHandler::HandleEstimateDataChunkSizeReq);
    // wbt
    _RegisterGlobalMetaReqHandler(MetaFsFileControlType::GetMetaFileInfoList,
                    &MetaVolumeHandler::HandleGetMetaFileInodeListReq);
    _RegisterGlobalMetaReqHandler(MetaFsFileControlType::GetFileInode,
                    &MetaVolumeHandler::HandleGetFileInodeReq);
}

int
MetaVolumeManager::_GetRequestHandlerIndex(MetaFsFileControlType reqType)
{
    int handlerIdx;
    if (reqType < MetaFsFileControlType::OnVolumeSpecificReq_Max)
    {
        handlerIdx = (int)reqType - (int)MetaFsFileControlType::OnVolumeSpcfReq_Base;
        assert(handlerIdx >= 0 && handlerIdx < (int)MetaFsFileControlType::OnVolumeSpcfReq_Count);
    }
    else
    {
        handlerIdx = (int)reqType - (int)MetaFsFileControlType::NonVolumeSpcfReq_Base;
        assert(handlerIdx >= 0 && handlerIdx < (int)MetaFsFileControlType::NonVolumeSpcfReq_Count);
    }

    return handlerIdx;
}

void
MetaVolumeManager::_RegisterVolumeSpcfReqHandler(MetaFsFileControlType reqType, MetaVolSpcfReqHandler handler)
{
    int handlerIdx = _GetRequestHandlerIndex(reqType);
    volumeSpcfReqHandler[handlerIdx] = handler;
}

void
MetaVolumeManager::_RegisterGlobalMetaReqHandler(MetaFsFileControlType reqType, GlobalMetaReqHandler handler)
{
    int handlerIdx = _GetRequestHandlerIndex(reqType);
    globalRequestHandler[handlerIdx] = handler;
}

POS_EVENT_ID
MetaVolumeManager::CheckReqSanity(MetaFsRequestBase& reqMsg)
{
    MetaFsFileControlRequest* msg = static_cast<MetaFsFileControlRequest*>(&reqMsg);

    if (false == msg->IsValid())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Given request is incorrect. Please check parameters.");

        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    return POS_EVENT_ID::SUCCESS;
}

void
MetaVolumeManager::InitVolume(MetaVolumeType volumeType, int arrayId, MetaLpnType maxVolPageNum)
{
    if (true == volContainer->IsGivenVolumeExist(volumeType))
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_MODULE_ALREADY_READY,
            "You attempted to init. volumeMgr duplicately for the given volume type={}",
            (uint32_t)volumeType);

        return;
    }

    volContainer->InitContext(volumeType, arrayId, maxVolPageNum, metaStorage);
    volHandler->InitHandler(volContainer);
}

bool
MetaVolumeManager::CreateVolume(MetaVolumeType volumeType)
{
    if (false == volContainer->CreateVolume(volumeType))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
            "Meta volume creation has been failed.");

        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta volume has been created (volumeType={})",
        (uint32_t)volumeType);

    return true;
}

bool
MetaVolumeManager::OpenVolume(bool isNPOR)
{
    bool isSuccess = volContainer->OpenAllVolumes(isNPOR);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED,
            "Meta volume open is failed. Volume is corrupted or isn't created yet");

        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "All volumes have been opened.");

    return true;
}

bool
MetaVolumeManager::CloseVolume(bool& resetCxt)
{
    bool isSuccess = volContainer->CloseAllVolumes(resetCxt /* output */);

    if (!isSuccess)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Meta volume close has been failed.");

        return false;
    }
    else
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta volume has been closed successfully.");

        return true;
    }
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeManager::_CheckRequest(MetaFsFileControlRequest& reqMsg)
{
    pair<MetaVolumeType, POS_EVENT_ID> rc = make_pair(reqMsg.volType, POS_EVENT_ID::SUCCESS);

    // based on reqMsg, select metaVolMgr to assign proper instance to handle the request
    switch (reqMsg.reqType)
    {
        case MetaFsFileControlType::FileCreate:
            if (0 == reqMsg.fileByteSize)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                    "The file size cannot be zero, fileSize={}", reqMsg.fileByteSize);
                rc.second = POS_EVENT_ID::MFS_INVALID_PARAMETER;
                break;
            }
            // fall-through

        case MetaFsFileControlType::GetAvailableSpace:
            rc = volContainer->DetermineVolumeToCreateFile(reqMsg.fileByteSize,
                                reqMsg.fileProperty, reqMsg.volType);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE,
                    "There is no NVRAM and SSD free space");
            }
            break;

        case MetaFsFileControlType::FileOpen:
            rc.second = volContainer->LookupMetaVolumeType(*reqMsg.fileName,
                                        reqMsg.volType);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                reqMsg.completionData.openfd = -1;

                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_OPENED,
                    "Cannot find \'{}\' file in array \'{}\'",
                    *reqMsg.fileName, reqMsg.arrayId);
            }
            break;

        case MetaFsFileControlType::FileDelete:
            rc.second = volContainer->LookupMetaVolumeType(*reqMsg.fileName,
                                        reqMsg.volType);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_DELETE_FAILED,
                    "Cannot find \'{}\' file", *reqMsg.fileName);
            }
            break;

        case MetaFsFileControlType::CheckFileExist:
            rc.second = volContainer->LookupMetaVolumeType(*reqMsg.fileName,
                                        reqMsg.volType);
            break;

        case MetaFsFileControlType::GetMaxMetaLpn:
            break;

        default:
            rc.second = volContainer->LookupMetaVolumeType(reqMsg.fd, reqMsg.volType);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                    "Cannot find the file, fd={}, request={}",
                    reqMsg.fd, reqMsg.reqType);
            }
            break;
    }

    return rc;
}

bool
MetaVolumeManager::_IsValidVolumeType(MetaVolumeType volType)
{
    return volType < MetaVolumeType::Max;
}

bool
MetaVolumeManager::_IsVolumeSpecificRequest(MetaFsFileControlType reqType)
{
    if (reqType < MetaFsFileControlType::OnVolumeSpecificReq_Max)
    {
        return true;
    }

    return false;
}

POS_EVENT_ID
MetaVolumeManager::ProcessNewReq(MetaFsRequestBase& reqMsg)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest* msg = static_cast<MetaFsFileControlRequest*>(&reqMsg);

    if (_IsVolumeSpecificRequest(msg->reqType))
    {
        if (!_IsValidVolumeType(msg->volType))
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                        "The volume type is invalid, fd={}, type={}",
                        msg->fd, msg->volType);
            return POS_EVENT_ID::MFS_INVALID_PARAMETER;
        }

        std::pair<MetaVolumeType, POS_EVENT_ID> checkVol = _CheckRequest(*msg);
        if (checkVol.second != POS_EVENT_ID::SUCCESS)
        {
            return checkVol.second;
        }

        rc = _ExecuteVolumeRequest(checkVol.first, *msg);
    }
    else
    {
        rc = _ExecuteGlobalMetaRequest(*msg);
    }

    return rc;
}

MetaVolSpcfReqHandler
MetaVolumeManager::_DispatchVolumeSpcfReqHandler(MetaFsFileControlType reqType)
{
    return volumeSpcfReqHandler[_GetRequestHandlerIndex(reqType)];
}

GlobalMetaReqHandler
MetaVolumeManager::_DispatchGlobalMetaReqHandler(MetaFsFileControlType reqType)
{
    return globalRequestHandler[_GetRequestHandlerIndex(reqType)];
}

POS_EVENT_ID
MetaVolumeManager::_ExecuteVolumeRequest(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    MetaVolSpcfReqHandler reqHandler = _DispatchVolumeSpcfReqHandler(reqMsg.reqType);

    return (volHandler->*reqHandler)(volType, reqMsg);
}

POS_EVENT_ID
MetaVolumeManager::_ExecuteGlobalMetaRequest(MetaFsFileControlRequest& reqMsg)
{
    GlobalMetaReqHandler reqHandler = _DispatchGlobalMetaReqHandler(reqMsg.reqType);

    return (volHandler->*reqHandler)(reqMsg);
}

POS_EVENT_ID
MetaVolumeManager::CheckFileAccessible(FileDescriptorType fd, MetaVolumeType volType)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::CheckFileAccessible;
    req.fd = fd;
    req.volType = volType;

    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        if (true == req.completionData.fileAccessible)
            return POS_EVENT_ID::SUCCESS;
        else
            return POS_EVENT_ID::MFS_FILE_INACTIVATED;
    }
    else
        return ret;
}

POS_EVENT_ID
MetaVolumeManager::GetFileSize(FileDescriptorType fd, MetaVolumeType volType,
    FileSizeType& outFileByteSize)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetFileSize;
    req.fd = fd;
    req.volType = volType;

    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outFileByteSize = req.completionData.fileSize;
    }
    else
    {
        outFileByteSize = 0;
    }

    return ret;
}

POS_EVENT_ID
MetaVolumeManager::GetDataChunkSize(FileDescriptorType fd, MetaVolumeType volType,
    FileSizeType& outDataChunkSize)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetDataChunkSize;
    req.fd = fd;
    req.volType = volType;

    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outDataChunkSize = req.completionData.dataChunkSize;
    }
    return ret;
}

POS_EVENT_ID
MetaVolumeManager::GetTargetMediaType(FileDescriptorType fd, MetaVolumeType volType,
    MetaStorageType& outTargetMediaType)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetTargetMediaType;
    req.fd = fd;
    req.volType = volType;
    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outTargetMediaType = req.completionData.targetMediaType;
    }
    return ret;
}

POS_EVENT_ID
MetaVolumeManager::GetFileBaseLpn(FileDescriptorType fd, MetaVolumeType volType,
    MetaLpnType& outFileBaseLpn)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetFileBaseLpn;
    req.fd = fd;
    req.volType = volType;
    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outFileBaseLpn = req.completionData.fileBaseLpn;
    }
    return ret;
}

void
MetaVolumeManager::SetMss(MetaStorageSubsystem* metaStorage)
{
    this->metaStorage = metaStorage;
}

MetaLpnType
MetaVolumeManager::GetTheLastValidLpn(MetaVolumeType volType)
{
    return volContainer->GetTheLastValidLpn(volType);
}
} // namespace pos
