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

#include "stripe_put_event.h"

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_wbstripe_allocator.h"

#if defined QOS_ENABLED_BE
#include "src/include/backend_event.h"
#endif

#include <string>

namespace pos
{
#if defined QOS_ENABLED_BE
StripePutEvent::StripePutEvent(Stripe& stripe, StripeId prevLsid, std::string& arrayName)
: Event(false, BackendEvent_Flush),
  stripe(stripe),
  prevLsid(prevLsid),
  iWBStripeAllocator(AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayName)),
  arrayName(arrayName)
{
    SetEventType(BackendEvent_Flush);
}
#else
StripePutEvent::StripePutEvent(Stripe& stripe, StripeId prevLsid, std::string& arrayName)
: Event(false),
  stripe(stripe),
  prevLsid(prevLsid),
  iWBStripeAllocator(AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayName)),
  arrayName(arrayName)
{
}
#endif

bool
StripePutEvent::Execute(void)
{
    if (stripe.IsOkToFree())
    {
        stripe.SetFinished(true);
        stripe.UnLinkReverseMap();
        iWBStripeAllocator->FreeWBStripeId(prevLsid);
        return true;
    }
    else
    {
        return false;
    }
}
} // namespace pos
