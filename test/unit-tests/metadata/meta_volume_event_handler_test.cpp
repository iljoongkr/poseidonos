#include "src/metadata/meta_volume_event_handler.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/allocator_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/journal_manager/log_write/i_journal_volume_event_handler_mock.h"
#include "test/unit-tests/mapper/i_mapper_volume_event_handler_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MetaVolumeEventHandler, MetaVolumeEventHandler_testIfConstructedSuccessfully)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);
}

TEST(MetaVolumeEventHandler, VolumeCreated_testIfCreateSuccess)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(mapper, VolumeCreated(volumeEvent.volId, volumeEvent.volSizeByte)).WillOnce(Return(true));

    bool result = handler.VolumeCreated(&volumeEvent, nullptr, nullptr);
    EXPECT_EQ(result, true);
}

TEST(MetaVolumeEventHandler, VolumeCreated_testIfCreateFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(mapper, VolumeCreated(volumeEvent.volId, volumeEvent.volSizeByte)).WillOnce(Return(false));

    bool result = handler.VolumeCreated(&volumeEvent, nullptr, nullptr);
    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeMounted_testIfMountSuccess)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(mapper, VolumeMounted(volumeEvent.volId, volumeEvent.volSizeByte)).WillOnce(Return(true));

    bool result = handler.VolumeMounted(&volumeEvent, nullptr, nullptr);
    EXPECT_EQ(result, true);
}

TEST(MetaVolumeEventHandler, VolumeMounted_testIfMountFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(mapper, VolumeMounted(volumeEvent.volId, volumeEvent.volSizeByte)).WillOnce(Return(false));

    bool result = handler.VolumeMounted(&volumeEvent, nullptr, nullptr);
    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeLoaded_testIfLoadSuccess)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(mapper, VolumeLoaded(volumeEvent.volId, volumeEvent.volSizeByte)).WillOnce(Return(true));

    bool result = handler.VolumeLoaded(&volumeEvent, nullptr, nullptr);
    EXPECT_EQ(result, true);
}

TEST(MetaVolumeEventHandler, VolumeLoaded_testIfLoadFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(mapper, VolumeLoaded(volumeEvent.volId, volumeEvent.volSizeByte)).WillOnce(Return(false));

    bool result = handler.VolumeLoaded(&volumeEvent, nullptr, nullptr);
    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeUpdated_testIfExecutedSuccessfully)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    bool result = handler.VolumeUpdated(&volumeEvent, nullptr, nullptr);
    EXPECT_EQ(result, true);
}

TEST(MetaVolumeEventHandler, VolumeUnmounted_testIfUnmountSuccess)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    {
        InSequence s;
        EXPECT_CALL(allocator, FinalizeActiveStripes(volumeEvent.volId)).WillOnce(Return(true));
        EXPECT_CALL(mapper, VolumeUnmounted(volumeEvent.volId, false)).WillOnce(Return(true));
    }

    bool result = handler.VolumeUnmounted(&volumeEvent, nullptr);
    EXPECT_EQ(result, true);
}

TEST(MetaVolumeEventHandler, VolumeUnmounted_testIfUnmountFailsWhenActiveStripeFlushFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(allocator, FinalizeActiveStripes(volumeEvent.volId)).WillOnce(Return(false));
    EXPECT_CALL(mapper, VolumeUnmounted(volumeEvent.volId, false)).Times(0);
    bool result = handler.VolumeUnmounted(&volumeEvent, nullptr);

    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeUnmounted_testIfUnmountFailsWhenVolumeMapUnmountFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(allocator, FinalizeActiveStripes(volumeEvent.volId)).WillOnce(Return(true));
    EXPECT_CALL(mapper, VolumeUnmounted(volumeEvent.volId, false)).WillOnce(Return(false));
    bool result = handler.VolumeUnmounted(&volumeEvent, nullptr);

    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeUnmounted_testIfMapFlushIsNotRequestedWhenJournalIsDisabled)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, nullptr);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(allocator, FinalizeActiveStripes(volumeEvent.volId)).WillOnce(Return(true));

    // flushMapRequired should be TRUE when journal disabled
    EXPECT_CALL(mapper, VolumeUnmounted(volumeEvent.volId, true)).WillOnce(Return(false));
    bool result = handler.VolumeUnmounted(&volumeEvent, nullptr);

    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeDeleted_testIfVolumeDeleteSuccessWhenJournalDisabled)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, nullptr);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    {
        InSequence s;
        EXPECT_CALL(mapper, PrepareVolumeDelete(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(mapper, DeleteVolumeMap(volumeEvent.volId)).WillOnce(Return(0));
    }

    int result = handler.VolumeDeleted(&volumeEvent, nullptr);
    EXPECT_EQ(result, true);
}

TEST(MetaVolumeEventHandler, VolumeDeleted_testIfVolumeDeleteSuccessWhenJournalEnabled)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    {
        InSequence s;
        EXPECT_CALL(mapper, PrepareVolumeDelete(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(journal, WriteVolumeDeletedLog(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(journal, TriggerMetadataFlush).WillOnce(Return(0));
        EXPECT_CALL(mapper, DeleteVolumeMap(volumeEvent.volId)).WillOnce(Return(0));
    }

    int result = handler.VolumeDeleted(&volumeEvent, nullptr);
    EXPECT_EQ(result, true);
}

TEST(MetaVolumeEventHandler, VolumeDeleted_testIfVolumeDeleteFailsWhenMapperPrepareFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    EXPECT_CALL(mapper, PrepareVolumeDelete(volumeEvent.volId)).WillOnce(Return(-1));
    EXPECT_CALL(journal, WriteVolumeDeletedLog(volumeEvent.volId)).Times(0);
    EXPECT_CALL(journal, TriggerMetadataFlush).Times(0);
    EXPECT_CALL(mapper, DeleteVolumeMap(volumeEvent.volId)).Times(0);

    int result = handler.VolumeDeleted(&volumeEvent, nullptr);
    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeDeleted_testIfVolumeDeleteFailsWhenJournalWriteFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    {
        InSequence s;
        EXPECT_CALL(mapper, PrepareVolumeDelete(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(journal, WriteVolumeDeletedLog(volumeEvent.volId)).WillOnce(Return(-1));
    }

    EXPECT_CALL(journal, TriggerMetadataFlush).Times(0);
    EXPECT_CALL(mapper, DeleteVolumeMap(volumeEvent.volId)).Times(0);

    int result = handler.VolumeDeleted(&volumeEvent, nullptr);
    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeDeleted_testIfVolumeDeleteFailsWhenMetaFlushFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    {
        InSequence s;
        EXPECT_CALL(mapper, PrepareVolumeDelete(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(journal, WriteVolumeDeletedLog(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(journal, TriggerMetadataFlush).WillOnce(Return(-1));
    }
    EXPECT_CALL(mapper, DeleteVolumeMap(volumeEvent.volId)).Times(0);

    int result = handler.VolumeDeleted(&volumeEvent, nullptr);
    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeDeleted_testIfVolumeDeleteFailsWhenDeleteVolumeMapFails)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    VolumeEventBase volumeEvent = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    {
        InSequence s;
        EXPECT_CALL(mapper, PrepareVolumeDelete(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(journal, WriteVolumeDeletedLog(volumeEvent.volId)).WillOnce(Return(0));
        EXPECT_CALL(journal, TriggerMetadataFlush).WillOnce(Return(0));
        EXPECT_CALL(mapper, DeleteVolumeMap(volumeEvent.volId)).WillOnce(Return(-1));
    }

    int result = handler.VolumeDeleted(&volumeEvent, nullptr);
    EXPECT_EQ(result, false);
}

TEST(MetaVolumeEventHandler, VolumeDetached_testIfMapDetachedSuccessfully)
{
    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    MetaVolumeEventHandler handler(&info, &mapper, &allocator, &journal);

    vector<int> volList = {1, 2, 3};
    EXPECT_CALL(mapper, VolumeDetached(volList)).Times(1);
    handler.VolumeDetached(volList, nullptr);
}

} // namespace pos
