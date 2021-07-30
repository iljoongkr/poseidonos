#include "src/metafs/metafs_file_intf.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_io_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_management_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_wbt_api_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include <gtest/gtest.h>
#include <string>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;

using namespace std;

namespace pos
{
class MetaFsFileIntfTester: public MetaFsFileIntf
{
public:
    MetaFsFileIntfTester(string fname, string aname, MetaFs* metaFs)
    : MetaFsFileIntf(fname, aname, metaFs)
    {
    }
    MetaFsFileIntfTester(string fname, int arrayId, MetaFs* metaFs)
    : MetaFsFileIntf(fname, arrayId, metaFs)
    {
    }

    int Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return _Read(fd, fileOffset, length, buffer);
    }

    int Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return _Write(fd, fileOffset, length, buffer);
    }

    void SetFileProperty(StorageOpt storageOpt)
    {
        _SetFileProperty(storageOpt);
    }

    MetaFilePropertySet& GetFileProperty()
    {
        return fileProperty;
    }
};

class MetaFsFileIntfFixture : public ::testing::Test
{
public:
    MetaFsFileIntfFixture(void)
    {
    }

    virtual ~MetaFsFileIntfFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        arrayInfo = new NiceMock<MockIArrayInfo>;
        mgmt = new NiceMock<MockMetaFsManagementApi>;
        ctrl = new NiceMock<MockMetaFsFileControlApi>;
        io = new NiceMock<MockMetaFsIoApi>;
        wbt = new NiceMock<MockMetaFsWBTApi>;

        metaFs = new MockMetaFs(arrayInfo, false, mgmt, ctrl, io, wbt);

        metaFile1 = new MetaFsFileIntfTester(fileName, arrayId, metaFs);
        metaFile2 = new MetaFsFileIntfTester(fileName, arrayName, metaFs);
    }

    virtual void
    TearDown(void)
    {
        delete metaFs;
        delete arrayInfo;
    }

protected:
    MockMetaFs* metaFs;
    NiceMock<MockIArrayInfo>* arrayInfo;
    NiceMock<MockMetaFsManagementApi>* mgmt;
    NiceMock<MockMetaFsFileControlApi>* ctrl;
    NiceMock<MockMetaFsIoApi>* io;
    NiceMock<MockMetaFsWBTApi>* wbt;

    MetaFsFileIntfTester* metaFile1;
    MetaFsFileIntfTester* metaFile2;

    string fileName = "TestFile";
    string arrayName = "TESTARRAY";
    int arrayId = 0;
    uint64_t fileSize = 100;
};

TEST_F(MetaFsFileIntfFixture, CreateMetaFsFile)
{
    EXPECT_CALL(*ctrl, Create).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile1->Create(fileSize), 0);
    EXPECT_EQ(metaFile2->Create(fileSize), 0);
}

TEST_F(MetaFsFileIntfFixture, OpenMetaFsFile)
{
    EXPECT_CALL(*ctrl, Open).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile1->Open(), 0);
    EXPECT_EQ(metaFile2->Open(), 0);
}

TEST_F(MetaFsFileIntfFixture, CloseMetaFsFile)
{
    EXPECT_CALL(*ctrl, Close).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile1->Close(), 0);
    EXPECT_EQ(metaFile2->Close(), 0);
}

TEST_F(MetaFsFileIntfFixture, DeleteMetaFsFile)
{
    EXPECT_CALL(*ctrl, Delete).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile1->Delete(), 0);
    EXPECT_EQ(metaFile2->Delete(), 0);
}

TEST_F(MetaFsFileIntfFixture, DoesFileExist)
{
    EXPECT_CALL(*ctrl, CheckFileExist).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_TRUE(metaFile1->DoesFileExist());
    EXPECT_TRUE(metaFile2->DoesFileExist());
}

TEST_F(MetaFsFileIntfFixture, GetFileSize)
{
    EXPECT_CALL(*ctrl, GetFileSize).WillRepeatedly(Return(fileSize));

    EXPECT_EQ(metaFile1->GetFileSize(), fileSize);
    EXPECT_EQ(metaFile2->GetFileSize(), fileSize);
}

TEST_F(MetaFsFileIntfFixture, IssueAsyncIO)
{
    EXPECT_CALL(*io, SubmitIO).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    AsyncMetaFileIoCtx ctx;

    EXPECT_EQ(metaFile1->AsyncIO(&ctx), 0);
}

TEST_F(MetaFsFileIntfFixture, CheckIoDoneStatus)
{
    MetaFsAioCbCxt* ctx = new MetaFsAioCbCxt(MetaFsIoOpcode::Read, 0, 0,
                                                nullptr, nullptr);

    EXPECT_NE(metaFile1->CheckIoDoneStatus(ctx), 0);
}

TEST_F(MetaFsFileIntfFixture, CheckStorage)
{
    EXPECT_EQ(metaFile1->GetStorage(), StorageOpt::SSD);
}

TEST_F(MetaFsFileIntfFixture, CheckFileProperty)
{
    StorageOpt opt = StorageOpt::NVRAM;
    MetaFilePropertySet& property = metaFile1->GetFileProperty();

    EXPECT_EQ(property.integrity, MetaFileIntegrityType::Default);
    EXPECT_EQ(property.ioAccPattern, MetaFileAccessPattern::Default);
    EXPECT_EQ(property.ioOpType, MetaFileDominant::Default);

    metaFile1->SetFileProperty(opt);

    EXPECT_EQ(property.integrity, MetaFileIntegrityType::Lvl0_Disable);
    EXPECT_EQ(property.ioAccPattern, MetaFileAccessPattern::ByteIntensive);
    EXPECT_EQ(property.ioOpType, MetaFileDominant::WriteDominant);
}

TEST_F(MetaFsFileIntfFixture, ReadMetaFile)
{
    EXPECT_CALL(*io, Read(_,_,_,_,_)).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile1->Read(0, 0, 0, nullptr), 0);
}

TEST_F(MetaFsFileIntfFixture, WriteMetaFile)
{
    EXPECT_CALL(*io, Write(_,_,_,_,_)).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile1->Write(0, 0, 0, nullptr), 0);
}
} // namespace pos
