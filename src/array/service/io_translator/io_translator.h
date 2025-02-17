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

#include <list>
#include <map>
#include <string>

#include "i_io_translator.h"
#include "i_translator.h"

using namespace std;

namespace pos
{
using ArrayTranslator = map<PartitionType, ITranslator*>;
class IOTranslator : public IIOTranslator
{
public:
    IOTranslator(void);
    virtual ~IOTranslator(void);
    int Translate(unsigned int arrayIndex, PartitionType part,
        PhysicalBlkAddr& dst, const LogicalBlkAddr& src) override;
    int ByteTranslate(unsigned int arrayIndex, PartitionType part,
        PhysicalByteAddr& dst, const LogicalByteAddr& src) override;
    int Convert(unsigned int arrayIndex, PartitionType part,
        list<PhysicalWriteEntry>& dst, const LogicalWriteEntry& src) override;
    int ByteConvert(unsigned int arrayIndex, PartitionType part,
        list<PhysicalByteWriteEntry>& dst, const LogicalByteWriteEntry& src) override;
    bool Register(unsigned int arrayIndex, ArrayTranslator trans);
    void Unregister(unsigned int arrayIndex);

private:
    ArrayTranslator* translators;
};
} // namespace pos
