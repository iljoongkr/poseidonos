#include "src/journal_manager/log_write/buffer_offset_allocator.h"
#include "src/include/pos_event_id.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/checkpoint/log_group_releaser_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"

using testing::NiceMock;
using ::testing::Return;

namespace pos
{
const std::string ARRAY_NAME = "POSArray";
const int numLogGroups = 2;
const uint64_t metaPageSize = 4032;
const uint64_t logBufferSize = 2 * metaPageSize * numLogGroups;

bool
IsCrossingMetaPages(uint64_t offset, int size)
{
    uint32_t startPage = offset / metaPageSize;
    uint32_t endPage = (offset + size - 1) / metaPageSize;

    return (startPage != endPage);
}

void
AllocateBuffer(BufferOffsetAllocator* allocator, uint64_t totalSizeToAllocate)
{
    uint64_t offset = 0;
    while (1)
    {
        int sizeToAllocate = std::rand() % 70 + 1;
        if (offset + sizeToAllocate < totalSizeToAllocate)
        {
            uint64_t allocatedOffset;
            int allocationResult = allocator->AllocateBuffer(sizeToAllocate, allocatedOffset);

            EXPECT_TRUE(allocationResult >= 0);

            if (allocationResult == 0)
            {
                EXPECT_TRUE(IsCrossingMetaPages(allocatedOffset, sizeToAllocate) == false);

                offset = allocatedOffset + sizeToAllocate;

                MapPageList dirty; // garbage data
                allocator->LogFilled(allocator->GetLogGroupId(allocatedOffset), dirty);
            }
            else if (allocationResult == (int)POS_EVENT_ID::JOURNAL_NO_LOG_BUFFER_AVAILABLE)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

TEST(BufferOffsetAllocator, Allocate_AllocateBuffer)
{
    // Given
    NiceMock<MockLogGroupReleaser>* releaser = new NiceMock<MockLogGroupReleaser>;
    NiceMock<MockJournalConfiguration>* config = new NiceMock<MockJournalConfiguration>;
    BufferOffsetAllocator* allocator = new BufferOffsetAllocator();

    // When : Request a buffer allocation with in the total size of log buffer
    ON_CALL(*config, GetNumLogGroups).WillByDefault(Return(numLogGroups));
    ON_CALL(*config, GetLogBufferSize).WillByDefault(Return(logBufferSize));
    ON_CALL(*config, GetLogGroupSize).WillByDefault(Return(logBufferSize / numLogGroups));
    ON_CALL(*config, GetMetaPageSize).WillByDefault(Return(metaPageSize));

    uint64_t startOffset = 0;
    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        LogGroupLayout groupLayout;
        groupLayout.startOffset = startOffset;
        groupLayout.maxOffset = startOffset + (logBufferSize / numLogGroups);
        groupLayout.footerStartOffset = groupLayout.maxOffset;
        ON_CALL(*config, GetLogBufferLayout(groupId)).WillByDefault(Return(groupLayout));
        startOffset = groupLayout.maxOffset;
    }
    allocator->Init(releaser, config);

    uint64_t totalSizeToAllocate = logBufferSize / numLogGroups;

    // Then : Allocator returns the offset that each log write requests can be written within one mpage on log buffer
    AllocateBuffer(allocator, totalSizeToAllocate);

    delete releaser;
    delete config;
    delete allocator;
}

TEST(BufferOffsetAllocator, Allocate_AllocateBufferWithCheckpoint)
{
    // Given
    NiceMock<MockLogGroupReleaser>* releaser = new NiceMock<MockLogGroupReleaser>;
    NiceMock<MockJournalConfiguration>* config = new NiceMock<MockJournalConfiguration>;
    BufferOffsetAllocator* allocator = new BufferOffsetAllocator();

    // When : Request a buffer allocation over the total size of log buffer
    ON_CALL(*config, GetNumLogGroups).WillByDefault(Return(numLogGroups));
    ON_CALL(*config, GetLogBufferSize).WillByDefault(Return(logBufferSize));
    ON_CALL(*config, GetLogGroupSize).WillByDefault(Return(logBufferSize / numLogGroups));
    ON_CALL(*config, GetMetaPageSize).WillByDefault(Return(metaPageSize));

    uint64_t startOffset = 0;
    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        LogGroupLayout groupLayout;
        groupLayout.startOffset = startOffset;
        groupLayout.maxOffset = startOffset + (logBufferSize / numLogGroups);
        groupLayout.footerStartOffset = groupLayout.maxOffset;
        ON_CALL(*config, GetLogBufferLayout(groupId)).WillByDefault(Return(groupLayout));
        startOffset = groupLayout.maxOffset;
    }
    allocator->Init(releaser, config);

    uint64_t totalSizeToAllocate = logBufferSize * 1.5;

    // Then : Allocator returns the offset that each log write requests can be written within log buffer
    AllocateBuffer(allocator, totalSizeToAllocate);

    delete releaser;
    delete config;
    delete allocator;
}
} // namespace pos
