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

#include "io_locker.h"

#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{

bool
IOLocker::Register(vector<ArrayDevice*> devList)
{
    group.AddDevice(devList);
    size_t prevSize = lockers.size();
    for (IArrayDevice* d : devList)
    {
        if (_Find(d) == nullptr)
        {
            IArrayDevice* m = group.GetMirror(d);
            if (m != nullptr)
            {
                StripeLocker* locker = new StripeLocker();
                lockers.emplace(d, locker);
                lockers.emplace(m, locker);
            }
        }
    }
    size_t newSize = lockers.size();
    POS_TRACE_INFO(POS_EVENT_ID::LOCKER_DEBUG_MSG, "IOLocker::Register, {} devs added, size: {} -> {}",
        devList.size(), prevSize, newSize);
    return true;
}

void
IOLocker::Unregister(vector<ArrayDevice*> devList)
{
    size_t prevSize = lockers.size();
    for (IArrayDevice* d : devList)
    {
        StripeLocker* locker = _Find(d);
        if (locker != nullptr)
        {
            lockers.erase(d);
            IArrayDevice* m = group.GetMirror(d);
            if (m != nullptr)
            {
                lockers.erase(m);
            }
            delete locker;
            locker = nullptr;
        }
    }
    group.RemoveDevice(devList);
    size_t newSize = lockers.size();
    POS_TRACE_INFO(POS_EVENT_ID::LOCKER_DEBUG_MSG, "IOLocker::Unregister, {} devs removed, size: {} -> {}",
        devList.size(), prevSize, newSize);
}

bool
IOLocker::TryBusyLock(IArrayDevice* dev, StripeId from, StripeId to)
{
    StripeLocker* locker = _Find(dev);
    if (locker == nullptr)
    {
        // TODO(SRM) expect a path that will not be reached
        POS_TRACE_WARN(POS_EVENT_ID::LOCKER_DEBUG_MSG, "IOLocker::TryBusyLock, no locker exists");
        return true;
    }

    return locker->TryBusyLock(from, to);
}

bool
IOLocker::TryLock(set<IArrayDevice*>& devs, StripeId val)
{
    set<StripeLocker*> lockersByGroup;
    for (IArrayDevice* d : devs)
    {
        StripeLocker* locker = _Find(d);
        if (locker == nullptr)
        {
            // TODO(SRM) expect a path that will not be reached
            POS_TRACE_WARN(POS_EVENT_ID::LOCKER_DEBUG_MSG, "IOLocker::TryLock, no locker exists");
            return true;
        }
        lockersByGroup.insert(locker);
    }
    int lockedCnt = 0;
    for (auto it = lockersByGroup.begin(); it != lockersByGroup.end(); ++it)
    {
        bool ret = (*it)->TryLock(val);
        if (ret == true)
        {
            lockedCnt++;
        }
        else
        {
            POS_TRACE_INFO(POS_EVENT_ID::LOCKER_DEBUG_MSG,
                "IOLocker::TryLock, failed to acquire entire locks for request, stripe:{}, total:{}, acquired:{}",
                val, lockersByGroup.size(), lockedCnt);
            while (lockedCnt > 0)
            {
                --it;
                (*it)->Unlock(val);
                lockedCnt--;
            }
            return false;
        }
    }
    return true;
}

void
IOLocker::Unlock(IArrayDevice* dev, StripeId val)
{
    StripeLocker* locker = _Find(dev);
    if (locker != nullptr)
    {
        locker->Unlock(val);
    }
    else
    {
        // TODO(SRM) expect a path that will not be reached
        POS_TRACE_WARN(POS_EVENT_ID::LOCKER_DEBUG_MSG, "IOLocker::Unlock, no locker exists");
    }
}

void
IOLocker::Unlock(set<IArrayDevice*>& devs, StripeId val)
{
    set<StripeLocker*> lockersByGroup;
    for (IArrayDevice* d : devs)
    {
        StripeLocker* locker = _Find(d);
        if (locker == nullptr)
        {
            // TODO(SRM) expect a path that will not be reached
            POS_TRACE_WARN(POS_EVENT_ID::LOCKER_DEBUG_MSG, "IOLocker::Unlock, no locker exists");
        }
        lockersByGroup.insert(locker);
    }
    for (StripeLocker* locker : lockersByGroup)
    {
        locker->Unlock(val);
    }
}

bool
IOLocker::ResetBusyLock(IArrayDevice* dev)
{
    StripeLocker* locker = _Find(dev);
    if (locker == nullptr)
    {
        // TODO(SRM) expect a path that will not be reached
        POS_TRACE_WARN(POS_EVENT_ID::LOCKER_DEBUG_MSG, "IOLocker::ResetBusyLock, no locker exists");
        return true;
    }

    return locker->ResetBusyLock();
}

StripeLocker*
IOLocker::_Find(IArrayDevice* dev)
{
    auto it = lockers.find(dev);
    if (it == lockers.end())
    {
        return nullptr;
    }

    return it->second;
}

} // namespace pos
