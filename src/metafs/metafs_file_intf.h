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

#include "src/metafs/metafs.h"
#include "src/metafs/include/metafs_aiocb_cxt.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/meta_file_intf/meta_file_intf.h"

#include <cassert>
#include <cstdint>
#include <string>

namespace pos
{

class MetaFsFileIntf : public MetaFileIntf
{
public:
    explicit MetaFsFileIntf(std::string fname, int arrayId,
                StorageOpt storageOpt = StorageOpt::DEFAULT);
    // only for test
    explicit MetaFsFileIntf(std::string fname, int arrayId, MetaFs* metaFs,
                StorageOpt storageOpt = StorageOpt::DEFAULT);
    virtual ~MetaFsFileIntf(void) override;

    virtual int Create(uint64_t fileSize) override;
    virtual bool DoesFileExist(void) override;
    virtual int Delete(void) override;
    virtual uint64_t GetFileSize(void) override;

    virtual int AsyncIO(AsyncMetaFileIoCtx* ctx) override;
    virtual int CheckIoDoneStatus(void* data) override;

    virtual int Open(void) override;
    virtual int Close(void) override;

protected:
    virtual void _SetFileProperty(StorageOpt storageOpt);
    virtual int _Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer) override;
    virtual int _Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer) override;

    MetaFs* metaFs;
};

} // namespace pos
