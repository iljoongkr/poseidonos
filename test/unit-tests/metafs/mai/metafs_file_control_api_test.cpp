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

#include "src/metafs/mai/metafs_file_control_api.h"
#include "test/unit-tests/metafs/mvm/meta_volume_manager_mock.h"

#include <gtest/gtest.h>

#include <vector>
#include <string>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MetaFsFileControlApi, WBT_testIfMetaFileInfoListCanBeReturned)
{
    int arrayId = 0;
    MetaVolumeType type = MetaVolumeType::SsdVolume;
    NiceMock<MockMetaVolumeManager>* volMgr = new NiceMock<MockMetaVolumeManager>;

    MetaFsFileControlApi api(arrayId, volMgr);

    std::vector<MetaFileInfoDumpCxt> result;

    EXPECT_CALL(*volMgr, CheckReqSanity).WillOnce(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*volMgr, ProcessNewReq).WillOnce(Return(POS_EVENT_ID::SUCCESS));

    result = api.Wbt_GetMetaFileList(type);
}

TEST(MetaFsFileControlApi, WBT_testIfMetaFileInodeCanBeReturned)
{
    int arrayId = 0;
    MetaVolumeType type = MetaVolumeType::SsdVolume;
    std::string fileName = "TEST_FILE";
    NiceMock<MockMetaVolumeManager>* volMgr = new NiceMock<MockMetaVolumeManager>;

    MetaFsFileControlApi api(arrayId, volMgr);

    MetaFileInodeInfo* result = nullptr;

    EXPECT_CALL(*volMgr, CheckReqSanity).WillOnce(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*volMgr, ProcessNewReq).WillOnce(Return(POS_EVENT_ID::SUCCESS));

    result = api.Wbt_GetMetaFileInode(fileName, type);
}

TEST(MetaFsFileControlApi, Get_testIfFileIoSizeCanBeRetrieved)
{
    int arrayId = 0;
    int fd = 0;
    StorageOpt type = StorageOpt::SSD;
    NiceMock<MockMetaVolumeManager>* volMgr = new NiceMock<MockMetaVolumeManager>;

    MetaFsFileControlApi api(arrayId, volMgr);
    api.SetStatus(true);

    EXPECT_CALL(*volMgr, CheckReqSanity).WillOnce(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*volMgr, ProcessNewReq).WillOnce(Return(POS_EVENT_ID::SUCCESS));

    size_t size = api.GetAlignedFileIOSize(fd, type);

    EXPECT_EQ(size, 0);
}
} // namespace pos
