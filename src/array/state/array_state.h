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

#ifndef ARRAY_STATE_H_
#define ARRAY_STATE_H_

#include "src/include/array_state_type.h"
#include "src/include/raid_state.h"
#include "src/state/interface/i_state_observer.h"
#include "src/state/interface/i_state_control.h"

#include <condition_variable>
#include <mutex>
#include <string>

namespace pos
{
class ArrayState : public IStateObserver
{
public:
    ArrayState(IStateControl* iState);
    virtual ~ArrayState(void);

    virtual void SetState(ArrayStateEnum nextState);
    virtual void SetLoad(RaidState rs);
    virtual void SetCreate(void);
    virtual void SetDelete(void);
    virtual bool SetRebuild(void);
    virtual void SetRebuildDone(bool isSuccess);
    virtual int SetMount(void);
    virtual int SetUnmount(void);
    virtual void SetDegraded(void);

    virtual int CanAddSpare(void);
    virtual int CanRemoveSpare(void);
    virtual int IsLoadable(void);
    virtual int IsCreatable(void);
    virtual int IsMountable(void);
    virtual int IsUnmountable(void);
    virtual int IsDeletable(void);
    virtual bool IsRebuildable(void);
    virtual bool IsRecoverable(void);
    virtual void RaidStateUpdated(RaidState rs);
    virtual bool Exists(void);
    virtual bool IsMounted(void);
    virtual bool IsBroken(void);
    virtual ArrayStateType GetState(void);
    virtual StateContext* GetSysState(void);

    virtual void StateChanged(StateContext* prev, StateContext* next) override;

private:
    void _SetState(ArrayStateEnum newState);
    bool _WaitState(StateContext* goal);

    IStateControl* iStateControl;

    StateContext* degradedState = nullptr;
    StateContext* rebuildingState = nullptr;
    StateContext* stopState = nullptr;

    std::mutex mtx;
    std::condition_variable cv;

    ArrayStateType state = ArrayStateEnum::NOT_EXIST;
};

} // namespace pos

#endif // ARRAY_STATE_H_
