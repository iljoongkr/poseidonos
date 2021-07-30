#include "src/metafs/mvm/volume/inode_table_header.h"
#include "src/metafs/config/metafs_config.h"
#include <gtest/gtest.h>

namespace pos
{
TEST(InodeTableHeader, CreateObject)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    InodeTableHeaderContent* content = header->GetContent();

    header->Create(100);

    EXPECT_EQ(content->totalInodeNum, 100);

    delete header;
}

TEST(InodeTableHeader, SetInUseFlag)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    InodeTableHeaderContent* content = header->GetContent();

    header->SetInodeInUse(1);

    EXPECT_EQ(content->inodeInUseBitmap.bits.count(), 1);
    EXPECT_EQ(content->inodeInUseBitmap.allocatedInodeCnt, 1);
    EXPECT_EQ(content->totalFileCreated, 1);

    delete header;
}

TEST(InodeTableHeader, ClearInUseFlag)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    InodeTableHeaderContent* content = header->GetContent();

    header->SetInodeInUse(1);

    EXPECT_EQ(content->inodeInUseBitmap.bits.count(), 1);
    EXPECT_EQ(content->inodeInUseBitmap.allocatedInodeCnt, 1);
    EXPECT_EQ(content->totalFileCreated, 1);

    header->ClearInodeInUse(1);

    EXPECT_EQ(content->inodeInUseBitmap.bits.count(), 0);
    EXPECT_EQ(content->inodeInUseBitmap.allocatedInodeCnt, 0);
    EXPECT_EQ(content->totalFileCreated, 0);

    delete header;
}

TEST(InodeTableHeader, CheckInUseFlag)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);

    header->SetInodeInUse(1);

    EXPECT_TRUE(header->IsFileInodeInUse(1));

    delete header;
}

TEST(InodeTableHeader, GetTotalCount)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    const int MAX_COUNT = 20;

    for (int i = 0; i < MAX_COUNT; ++i)
    {
        header->SetInodeInUse(i);
    }

    EXPECT_EQ(header->GetTotalAllocatedInodeCnt(), MAX_COUNT);

    delete header;
}

TEST(InodeTableHeader, CheckBuilingFreeInode)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    const int MAX_COUNT = 5;

    // set used inode idx, 0 to 4
    for (int i = 0; i < MAX_COUNT; ++i)
    {
        header->SetInodeInUse(i);
    }

    header->BuildFreeInodeEntryMap();

    // get unused inode idx, 5 to 9
    for (int i = 0; i < MAX_COUNT; ++i)
    {
        EXPECT_EQ(header->GetFreeInodeEntryIdx(), i + MAX_COUNT);
    }

    delete header;
}

TEST(InodeTableHeader, CheckBitset)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    const int MAX_COUNT = 5;

    for (int i = 0; i < MAX_COUNT; ++i)
    {
        header->SetInodeInUse(i);
    }

    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& bits = header->GetInodeInUseBitmap();

    EXPECT_EQ(bits.count(), MAX_COUNT);

    delete header;
}

TEST(InodeTableHeader, CheckExtents)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    const int FIXED_SIZE = 5;
    const int MAX_COUNT = 30;
    std::vector<MetaFileExtent> extents, copy;

    for (int i = 0; i < MAX_COUNT; ++i)
    {
        extents.push_back({(MetaLpnType)(i * FIXED_SIZE + 1), (MetaLpnType)FIXED_SIZE});
    }

    header->SetFileExtentContent(extents);

    copy = header->GetFileExtentContent();

    // both of them have to be same
    EXPECT_EQ(extents.size(), copy.size());

    for (int i = 0; i < copy.size(); ++i)
    {
        EXPECT_EQ(copy[i].GetStartLpn(), i * FIXED_SIZE + 1);
        EXPECT_EQ(copy[i].GetCount(), FIXED_SIZE);
    }

    delete header;
}

TEST(InodeTableHeader, CheckExtentsMaxSize)
{
    InodeTableHeader* header = new InodeTableHeader(MetaVolumeType::SsdVolume, 0);
    InodeTableHeaderContent* content = header->GetContent();

    EXPECT_EQ(header->GetFileExtentContentSize(), content->allocExtentsList.size());

    delete header;
}

} // namespace pos
