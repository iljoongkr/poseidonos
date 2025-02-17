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

#pragma once

#include <condition_variable>
#include <string>
#include <vector>
#include "metafs_io_q.h"
#include "metafs_return_code.h"
#include "metafs_io_request.h"
#include "mim_state.h"
#include "mpio.h"
#include "mpio_list_context.h"
#include "mpio_pool.h"
#include "metafs_aiocb_cxt.h"

namespace pos
{
#define AsMioStateEntryPoint(funcPointer, obj) AsEntryPointParam1(funcPointer, obj)

using MpioDonePollerCb = std::function<void(void)>;
using MioAsyncDoneCb = AsyncCallback;

class MioStateExecuteEntry
{
public:
    using MioStateHandler = std::function<bool(MioState)>;

    MioStateExecuteEntry(void) = delete;
    MioStateExecuteEntry(MioState state, MioStateHandler handler, MioState expNextState)
    {
        this->state = state;
        this->handler = handler;
        this->expNextState = expNextState;
    }
    ~MioStateExecuteEntry(void)
    {
    }
    MioState
    GetState(void)
    {
        return state;
    }
    MioState
    GetExpNextState(void)
    {
        return expNextState;
    }
    bool
    DispatchHandler(MioState expNextState)
    {
        return handler(expNextState);
    }

private:
    MioState state;
    MioStateHandler handler;
    MioState expNextState;
};

class Mio : public MetaAsyncRunnable<MetaAsyncCbCxt, MioState, MioStateExecuteEntry>
{
public:
    Mio(void) = delete;
    explicit Mio(MpioPool* mpioPool);
    Mio(const Mio& mio) = delete;
    Mio& operator=(const Mio& mio) = delete;

    virtual ~Mio(void);

    virtual void InitStateHandler(void) override;
    virtual void Setup(MetaFsIoRequest* ioReq, MetaLpnType baseLpn, MetaStorageSubsystem* metaStorage);
    virtual void Reset(void);

    virtual void SetMpioDoneNotifier(PartialMpioDoneCb& partialMpioDoneHandler);
    virtual void SetMpioDonePoller(MpioDonePollerCb& mpioDonePoller);

    virtual void SetIoCQ(MetaFsIoQ<Mio*>* ioCQ);

    void NotifiyPartialMpioDone(Mpio* mpio);
    virtual bool IsSyncIO(void);

    virtual bool IsRead(void);
    virtual MetaLpnType GetStartLpn(void);
    virtual bool IsTargetStorageSSD(void);
    virtual bool Init(MioState expNextState = MioState::Max);
    virtual bool Issue(MioState expNextState = MioState::Max);
    virtual bool Complete(MioState expNextState = MioState::Max);
    virtual MfsError GetError(void);
    virtual void* GetClientAioCbCxt(void);
    void NotifyCompletionToClient(void);
    virtual void SetLocalAioCbCxt(MioAsyncDoneCb& callback);
    virtual int GetArrayId(void);

#if MPIO_CACHE_EN
    virtual void SetMergedRequestList(std::vector<MetaFsIoRequest*>* list);
    virtual void ClearMergedRequestList(void);
    virtual std::vector<MetaFsIoRequest*>* GetMergedRequestList(void);
#endif

protected:
    void _BindMpioPool(MpioPool* mpioPool);
    void _BuildMpioMap(void);
    void _PrepareMpioInfo(MpioIoInfo& mpioIoInfo,
        MetaLpnType lpn, FileSizeType byteOffset, FileSizeType byteSize, FileBufType buf,
        MetaLpnType lpnCnt, uint32_t mpio_id);
    void _MarkMpioComplete(Mpio& mpio);
    void _FinalizeMpio(Mpio& mpio);
    void _NotifyIoCompletionToClient(void);
    Mpio* _AllocMpio(MpioIoInfo& mpioIoInfo, bool partialIO);
    void _HandleMpioDone(void* data);
    MpioType _LookupMpioType(MetaIoRequestType type);
    MetaLpnType _GetCalculateStartLpn(MetaFsIoRequest* ioReq);

    MetaFsIoRequest* originReq;
    MetaIoOpcode opCode;
    MpioListContext mpioListCxt;
    uint32_t fileDataChunkSize;
    MetaLpnType startLpn;
    MfsError error;
    MetaFsIoQ<Mio*>* ioCQ;
    MpioPool* mpioPool;
    MetaAsyncCbCxt aioCbCxt;
#if MPIO_CACHE_EN
    std::vector<MetaFsIoRequest*>* mergedRequestList = nullptr;
#endif

    static const MetaIoOpcode ioOpcodeMap[static_cast<uint32_t>(MetaIoRequestType::Max)];
    MetaFsSpinLock mpioListCxtLock;
    std::mutex mioDoneLock;

    PartialMpioDoneCb partialMpioDoneNotifier;
    MpioDonePollerCb mpioDonePoller;
    MpioAsyncDoneCb mpioAsyncDoneCallback;

    MetaStorageSubsystem* metaStorage = nullptr;
};

extern InstanceTagIdAllocator mioTagIdAllocator;
} // namespace pos
