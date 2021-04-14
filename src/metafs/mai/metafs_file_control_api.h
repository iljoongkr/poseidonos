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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta File Management API
*/

#pragma once

#include <string>

#include "metafs_adapter_include.h"

namespace pos
{
class MetaFsFileControlApi
{
public:
    virtual MetaFsReturnCode<POS_EVENT_ID> CreateVolume(std::string& fileName, std::string& arrayName, uint64_t fileByteSize,
        MetaFilePropertySet prop = MetaFilePropertySet());
    virtual MetaFsReturnCode<POS_EVENT_ID> Delete(std::string& fileName, std::string& arrayName);
    virtual MetaFsReturnCode<POS_EVENT_ID> Open(std::string& fileName, std::string& arrayName);
    virtual MetaFsReturnCode<POS_EVENT_ID> Close(FileDescriptorType fd, std::string& arrayName);
    virtual MetaFsReturnCode<POS_EVENT_ID> CheckFileExist(std::string& fileName, std::string& arrayName);
    virtual size_t GetFileSize(int fd, std::string& arrayName);
    virtual size_t GetAlignedFileIOSize(int fd, std::string& arrayName);
    virtual size_t EstimateAlignedFileIOSize(MetaFilePropertySet& prop, std::string& arrayName);
    virtual size_t GetTheBiggestExtentSize(MetaFilePropertySet& prop, std::string& arrayName);

private:
    MetaFsVolumeManager volMgr;
};
} // namespace pos
