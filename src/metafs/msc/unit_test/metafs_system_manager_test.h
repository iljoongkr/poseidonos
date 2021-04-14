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

#ifndef __UT_METAFS_SYSTEM_MANAGER_TEST_H__
#define __UT_METAFS_SYSTEM_MANAGER_TEST_H__

#include <string>
#include "gtest/gtest.h"
#include "metafs.h"
#include "metafs_system_manager.h"
#include "msc_test.h"

namespace pos
{
class MetaFsSystemManagerTest : public ::testing::Test
{
public:
    void
    SetUp(void) override
    {
        MetaStorageMediaInfoList mediaInfoList;
        MetaStorageInfo ssdInfo;
        ssdInfo.media = MetaStorageType::SSD;
        ssdInfo.mediaCapacity = (uint64_t)5 * 1024 * 1024 * 1024;
        mediaInfoList.push_back(ssdInfo);
        MetaStorageInfo nvramInfo;
        nvramInfo.media = MetaStorageType::NVRAM;
        nvramInfo.mediaCapacity = (uint64_t)1 * 1024 * 1024 * 1024;
        mediaInfoList.push_back(nvramInfo);

        std::string arrayName = "POSArray";

        metaFs.Init(arrayName, mediaInfoList);
    }

    void
    TearDown(void) override
    {
    }
};
} // namespace pos

#endif // __UT_METAFS_SYSTEM_MANAGER_TEST_H__
