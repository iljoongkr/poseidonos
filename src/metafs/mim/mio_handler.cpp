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

#include "mio_handler.h"
#include <string>
#include <utility>
#include "Air.h"
#include "src/metafs/include/metafs_service.h"
#include "metafs_aiocb_cxt.h"
#include "mfs_async_runnable_template.h"
#include "metafs_log.h"
#include "metafs_mutex.h"
#include "meta_volume_manager.h"
#include "src/metafs/storage/mss.h"
#include "src/event_scheduler/event.h"

#if defined(IBOFOS_BACKEND_IO)
#include "metafs_aio_completer.h"
#endif

namespace pos
{
MioHandler::MioHandler(int threadId, int coreId, int coreCount)
: ioSQ(nullptr),
  ioCQ(nullptr),
  cpuStallCnt(0)
{
    ioCQ = new MetaFsIoQ<Mio*>();
    ioSQ = new MetaFsIoQ<MetaFsIoRequest*>();

    std::string cqName("IoCQ-" + std::to_string(coreId));
    ioCQ->Init(cqName.c_str(), MAX_CONCURRENT_MIO_PROC_THRESHOLD);

    mpioPool = new MpioPool(MAX_CONCURRENT_MIO_PROC_THRESHOLD);
    mioPool = new MioPool(mpioPool, MAX_CONCURRENT_MIO_PROC_THRESHOLD);

    mioCompletionCallback = AsEntryPointParam1(&MioHandler::_HandleMioCompletion, this);

    this->bottomhalfHandler = nullptr;
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "mio handler constructed. threadId={}, coreId={}",
        threadId, coreId);
}

MioHandler::MioHandler(int threadId, int coreId, MetaFsIoQ<MetaFsIoRequest*>* ioSQ,
        MetaFsIoQ<Mio*>* ioCQ, MpioPool* mpioPool, MioPool* mioPool)
: ioSQ(ioSQ),
  ioCQ(ioCQ),
  mioPool(mioPool),
  mpioPool(mpioPool),
  cpuStallCnt(0)
{
    std::string cqName("IoCQ-" + std::to_string(coreId));
    ioCQ->Init(cqName.c_str(), MAX_CONCURRENT_MIO_PROC_THRESHOLD);

    mioCompletionCallback = AsEntryPointParam1(&MioHandler::_HandleMioCompletion, this);

    this->bottomhalfHandler = nullptr;
}

MioHandler::~MioHandler(void)
{
    delete mpioPool;
    delete mioPool;

    for (uint32_t index = 0; index < MetaFsConfig::MAX_ARRAY_CNT; index++)
    {
        for (uint32_t storage = 0; storage < NUM_STORAGE; storage++)
        {
            if (nullptr != ioRangeOverlapChker[index][storage])
            {
                delete ioRangeOverlapChker[index][storage];
            }
        }
    }

    if (nullptr != ioCQ)
        delete ioCQ;

    if (nullptr != ioSQ)
        delete ioSQ;
}

void
MioHandler::BindPartialMpioHandler(MpioHandler* ptMpioHandler)
{
    this->bottomhalfHandler = ptMpioHandler;
    this->bottomhalfHandler->BindMpioPool(mpioPool);

    partialMpioDoneNotifier = AsEntryPointParam1(&MpioHandler::EnqueuePartialMpio, bottomhalfHandler);
    mpioDonePoller = AsEntryPointNoParam(&MpioHandler::BottomhalfMioProcessing, bottomhalfHandler);
}

void
MioHandler::_HandleIoSQ(void)
{
    // if mio is not available, no new request can be serviced.
    if (mioPool->IsEmpty())
    {
        return;
    }

    MetaFsIoRequest* reqMsg = ioSQ->Dequeue();
    if (nullptr == reqMsg)
    {
        if (cpuStallCnt++ > 1000)
        {
            usleep(1);
            cpuStallCnt = 0;
        }
        return;
    }
    cpuStallCnt = 0;

#if RANGE_OVERLAP_CHECK_EN // range overlap enable
    if (_IsRangeOverlapConflicted(reqMsg) || _IsPendedRange(reqMsg))
    {
        _PushToRetry(reqMsg);
        return;
    }
#endif

    Mio* mio = DispatchMio(*reqMsg);
    if (nullptr == mio)
    {
        EnqueueNewReq(reqMsg);
        return;
    }

#if RANGE_OVERLAP_CHECK_EN // range overlap enable
    _RegisterRangeLockInfo(reqMsg);
#endif
    ExecuteMio(*mio);
}

void
MioHandler::_HandleIoCQ(void)
{
    Mio* mio = ioCQ->Dequeue();
    if (mio)
    {
        while (mio->IsCompleted() != true)
        {
        }

#if RANGE_OVERLAP_CHECK_EN // range overlap enable
        _FreeLockContext(mio);
        _DiscoverIORangeOverlap(); // find other pending I/O
#endif
        if (true == mio->IsSyncIO())
        {
            mio->NotifyCompletionToClient();
        }
        _FinalizeMio(mio);
    }
}

void
MioHandler::_DiscoverIORangeOverlap(void)
{
    for (auto it = pendingIoRetryQ.begin(); it != pendingIoRetryQ.end();)
    {
        MetaFsIoRequest* pendingIoReq = it->second;

        if (!_IsRangeOverlapConflicted(pendingIoReq))
        {
#if MPIO_CACHE_EN
            if (MetaStorageType::NVRAM == pendingIoReq->targetMediaType)
            {
                if (!_ExecutePendedIo(pendingIoReq))
                    break;
            }
            else
#endif
            {
                Mio* mio = DispatchMio(*pendingIoReq);
                if (nullptr == mio)
                    return;

                pendingIoRetryQ.erase(it);

                MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                    "[Msg ][EraseRetryQ] type={}, req.tagId={}, fileOffset={}, Lpn={}, numPending={}",
                    pendingIoReq->reqType, pendingIoReq->tagId, pendingIoReq->byteOffsetInFile,
                    pendingIoReq->byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE,
                    pendingIoRetryQ.size());

                _RegisterRangeLockInfo(pendingIoReq);
                ExecuteMio(*mio);
            }
        }

        it = pendingIoRetryQ.upper_bound(pendingIoReq->baseMetaLpn);
    }
}

#if MPIO_CACHE_EN
bool
MioHandler::_ExecutePendedIo(MetaFsIoRequest* reqMsg)
{
    Mio* mio = DispatchMio(*reqMsg);
    if (nullptr == mio)
        return false;

    _RegisterRangeLockInfo(reqMsg);


    std::vector<MetaFsIoRequest*>* reqList = new std::vector<MetaFsIoRequest*>();
    auto range = pendingIoRetryQ.equal_range(reqMsg->baseMetaLpn);

    for (auto it = range.first; it != range.second;)
    {
        MetaFsIoRequest* msg = it->second;
        if (reqMsg->arrayId == msg->arrayId)
        {
            reqList->push_back(msg);
            pendingIoRetryQ.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    mio->SetMergedRequestList(reqList);

    ExecuteMio(*mio);

    return true;
}
#endif

bool
MioHandler::_IsPendedRange(MetaFsIoRequest* reqMsg)
{
    auto it = pendingIoRetryQ.find(reqMsg->baseMetaLpn);

    if (it == pendingIoRetryQ.end())
    {
        return false;
    }
    else
    {
        for (; it != pendingIoRetryQ.end(); ++it)
        {
            if (reqMsg->targetMediaType == it->second->targetMediaType)
            {
                return true;
            }
        }
    }

    return false;
}

void
MioHandler::TophalfMioProcessing(void)
{
    _HandleIoSQ();
    _HandleIoCQ();
}

bool
MioHandler::EnqueueNewReq(MetaFsIoRequest* reqMsg)
{
    if (false == ioSQ->Enqueue(reqMsg))
    {
        return false;
    }
    return true;
}

void
MioHandler::_FinalizeMio(Mio* mio)
{
    mioPool->Release(mio);
}

Mio*
MioHandler::_AllocNewMio(MetaFsIoRequest& reqMsg)
{
    Mio* mio = mioPool->Alloc();

    if (nullptr == mio)
        return nullptr;

    MetaLpnType fileBaseLpn = reqMsg.fileCtx->fileBaseLpn;

    mio->Setup(&reqMsg, fileBaseLpn, MetaFsServiceSingleton::Instance()->GetMetaFs(reqMsg.arrayId)->GetMss());

    if (false == mio->IsSyncIO())
    {
        mio->SetLocalAioCbCxt(mioCompletionCallback);
    }

    mio->SetMpioDoneNotifier(partialMpioDoneNotifier);
    mio->SetMpioDonePoller(mpioDonePoller);
    mio->SetIoCQ(ioCQ);

    reqMsg.targetMediaType = reqMsg.fileCtx->storageType;

    return mio;
}

Mio*
MioHandler::DispatchMio(MetaFsIoRequest& reqMsg)
{
    Mio* mio = _AllocNewMio(reqMsg);

    return mio;
}

void
MioHandler::_PushToRetry(MetaFsIoRequest* reqMsg)
{
    reqMsg->SetRetryFlag();
    pendingIoRetryQ.insert(pair<MetaLpnType, MetaFsIoRequest*>(reqMsg->baseMetaLpn, reqMsg));

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Msg ][_PushToRetry] type={}, req.tagId={}, ByteOffset={}, Lpn={}, numPending={}",
        reqMsg->reqType, reqMsg->tagId, reqMsg->byteOffsetInFile,
        reqMsg->byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE,
        pendingIoRetryQ.size());
}

bool
MioHandler::_IsRangeOverlapConflicted(MetaFsIoRequest* reqMsg)
{
    if (reqMsg->reqType == MetaIoRequestType::Read)
    {
        return false;
    }

    int storage = (int)reqMsg->targetMediaType;
    int arrayId = reqMsg->arrayId;

    if (nullptr != ioRangeOverlapChker[arrayId][storage])
        return ioRangeOverlapChker[arrayId][storage]->IsRangeOverlapConflicted(reqMsg);

    return false;
}

void
MioHandler::_RegisterRangeLockInfo(MetaFsIoRequest* reqMsg)
{
    int storage = (int)reqMsg->targetMediaType;
    int arrayId = reqMsg->arrayId;

    if (nullptr != ioRangeOverlapChker[arrayId][storage])
        ioRangeOverlapChker[arrayId][storage]->PushReqToRangeLockMap(reqMsg);
}

void
MioHandler::_FreeLockContext(Mio* mio)
{
    int storage = mio->IsTargetStorageSSD() ? (int)MetaStorageType::SSD : (int)MetaStorageType::NVRAM;
    int arrayId = mio->GetArrayId();

    if (nullptr != ioRangeOverlapChker[arrayId][storage])
        ioRangeOverlapChker[arrayId][storage]->FreeLockContext(mio->GetStartLpn(), mio->IsRead());
}

void
MioHandler::ExecuteMio(Mio& mio)
{
    mio.ExecuteAsyncState();
}

bool
MioHandler::AddArrayInfo(int arrayId)
{
    assert(arrayId < (int)MetaFsConfig::MAX_ARRAY_CNT);

    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayId);

    for (uint32_t storage = 0; storage < NUM_STORAGE; storage++)
    {
        size_t maxLpn = metaFs->ctrl->GetMaxMetaLpn(static_cast<MetaVolumeType>(storage));

        ioRangeOverlapChker[arrayId][storage] = new MetaFsIoRangeOverlapChker();
        ioRangeOverlapChker[arrayId][storage]->Init(maxLpn);
    }

    return true;
}

bool
MioHandler::RemoveArrayInfo(int arrayId)
{
    assert(arrayId < (int)MetaFsConfig::MAX_ARRAY_CNT);

    bool result = true;

    for (uint32_t storage = 0; storage < NUM_STORAGE; storage++)
    {
        if (nullptr == ioRangeOverlapChker[arrayId][storage])
        {
            result = false;
            continue;
        }

        delete ioRangeOverlapChker[arrayId][storage];
        ioRangeOverlapChker[arrayId][storage] = nullptr;
    }

    return result;
}

void
MioHandler::_HandleMioCompletion(void* data)
{
    // FIXME: will replace to handle below logic by mio completion event handler
    Mio* mio = reinterpret_cast<Mio*>(data);
    assert(mio->IsSyncIO() == false);

#if MPIO_CACHE_EN
    std::vector<MetaFsIoRequest*>* reqList = mio->GetMergedRequestList();
    if (nullptr != reqList)
    {
        for (auto it : *reqList)
        {
            MetaFsAioCbCxt* aiocb = reinterpret_cast<MetaFsAioCbCxt*>(it->aiocb);

            if (aiocb)
            {
                aiocb->SetErrorStatus(mio->GetError());
                _SendAioDoneEvent(aiocb);
            }
        }
    }
    else
#endif
    {
        MetaFsAioCbCxt* aiocb = reinterpret_cast<MetaFsAioCbCxt*>(mio->GetClientAioCbCxt());

        if (aiocb)
        {
            aiocb->SetErrorStatus(mio->GetError());
            _SendAioDoneEvent(aiocb);
        }
    }
}

void
MioHandler::_SendAioDoneEvent(void* aiocb)
{
    // create event for callback by event worker
#if defined(IBOFOS_BACKEND_IO)
    MetaFsAioCompleter* event =
        new MetaFsAioCompleter(static_cast<MetaFsAioCbCxt*>(aiocb));
    pos::EventSchedulerSingleton::Instance()->EnqueueEvent(event);
#else
    (reinterpret_cast<MetaFsAioCbCxt*>(aiocb))->InvokeCallback();
#endif
}
} // namespace pos
