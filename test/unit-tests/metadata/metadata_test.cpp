#include "src/metadata/metadata.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/allocator_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/array_models/interface/i_mount_sequence_mock.h"
#include "test/unit-tests/journal_manager/journal_manager_mock.h"
#include "test/unit-tests/mapper/mapper_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(Metadata, Metadata_testContructor)
{
    // Given 1
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIStateControl> stateControl;

    // When 1
    Metadata metaForProductCode(nullptr, &arrayInfo, &stateControl);

    // Given 2
    NiceMock<MockMapper>* mapper = new NiceMock<MockMapper>(&arrayInfo, &stateControl);
    NiceMock<MockAllocator>* allocator = new NiceMock<MockAllocator>(nullptr, &arrayInfo, &stateControl);
    NiceMock<MockJournalManager>* journal = new NiceMock<MockJournalManager>(&arrayInfo, &stateControl);

    // When 2
    Metadata metaForUt(&arrayInfo, mapper, allocator, journal);
}

// Disabled to avoid "segfault" at metafs_file_intf.cpp:161.
// TODO(yyu): follow up with munseop.lim
TEST(Metadata, DISABLED_Init_testIfEverySequenceIsInitialized)
{
    // Given
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIStateControl> stateControl;
    NiceMock<MockMapper>* mapper = new NiceMock<MockMapper>(&arrayInfo, &stateControl);
    NiceMock<MockAllocator>* allocator = new NiceMock<MockAllocator>(nullptr, &arrayInfo, &stateControl);
    NiceMock<MockJournalManager>* journal = new NiceMock<MockJournalManager>(&arrayInfo, &stateControl);

    ON_CALL(arrayInfo, GetName).WillByDefault(Return("POSAarray"));
    ON_CALL(arrayInfo, GetIndex).WillByDefault(Return(0));

    Metadata meta(&arrayInfo, mapper, allocator, journal);

    EXPECT_CALL(*mapper, Init).WillOnce(Return(0));
    EXPECT_CALL(*allocator, Init).WillOnce(Return(0));
    EXPECT_CALL(*journal, Init).WillOnce(Return(0));

    // When
    int actual = meta.Init();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(Metadata, Init_testIfMapperIsRolledBack)
{
    // Given
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIStateControl> stateControl;
    NiceMock<MockMapper>* mapper = new NiceMock<MockMapper>(&arrayInfo, &stateControl);
    NiceMock<MockAllocator>* allocator = new NiceMock<MockAllocator>(nullptr, &arrayInfo, &stateControl);
    NiceMock<MockJournalManager>* journal = new NiceMock<MockJournalManager>(&arrayInfo, &stateControl);

    Metadata meta(&arrayInfo, mapper, allocator, journal);

    int MAPPER_FAILURE = 123;
    EXPECT_CALL(*mapper, Init).WillOnce(Return(MAPPER_FAILURE));
    EXPECT_CALL(*mapper, Dispose).Times(1);

    // When
    int actual = meta.Init();

    // Then
    ASSERT_EQ(MAPPER_FAILURE, actual);
}

TEST(Metadata, Init_testIfMapperAndAllocatorAreRolledBack)
{
    // Given
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIStateControl> stateControl;
    NiceMock<MockMapper>* mapper = new NiceMock<MockMapper>(&arrayInfo, &stateControl);
    NiceMock<MockAllocator>* allocator = new NiceMock<MockAllocator>(nullptr, &arrayInfo, &stateControl);
    NiceMock<MockJournalManager>* journal = new NiceMock<MockJournalManager>(&arrayInfo, &stateControl);

    Metadata meta(&arrayInfo, mapper, allocator, journal);

    int ALLOCATOR_FAILURE = 456;
    EXPECT_CALL(*mapper, Init).WillOnce(Return(0));
    EXPECT_CALL(*allocator, Init).WillOnce(Return(ALLOCATOR_FAILURE));
    EXPECT_CALL(*allocator, Dispose).Times(1);
    EXPECT_CALL(*mapper, Dispose).Times(1);

    // When
    int actual = meta.Init();

    // Then
    ASSERT_EQ(ALLOCATOR_FAILURE, actual);
}

TEST(Metadata, Init_testIfMapperAndAllocatorAndJournalAreRolledBack)
{
    // Given
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIStateControl> stateControl;
    NiceMock<MockMapper>* mapper = new NiceMock<MockMapper>(&arrayInfo, &stateControl);
    NiceMock<MockAllocator>* allocator = new NiceMock<MockAllocator>(nullptr, &arrayInfo, &stateControl);
    NiceMock<MockJournalManager>* journal = new NiceMock<MockJournalManager>(&arrayInfo, &stateControl);

    Metadata meta(&arrayInfo, mapper, allocator, journal);

    int JOURNAL_FAILURE = 456;
    EXPECT_CALL(*mapper, Init).WillOnce(Return(0));
    EXPECT_CALL(*allocator, Init).WillOnce(Return(0));
    EXPECT_CALL(*journal, Init).WillOnce(Return(JOURNAL_FAILURE));
    EXPECT_CALL(*journal, Dispose).Times(1);
    EXPECT_CALL(*allocator, Dispose).Times(1);
    EXPECT_CALL(*mapper, Dispose).Times(1);

    // When
    int actual = meta.Init();

    // Then
    ASSERT_EQ(JOURNAL_FAILURE, actual);
}

TEST(Metadata, Dispose_testIfAllSequenceInvokeDispose)
{
    // Given
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIStateControl> stateControl;
    NiceMock<MockMapper>* mapper = new NiceMock<MockMapper>(&arrayInfo, &stateControl);
    NiceMock<MockAllocator>* allocator = new NiceMock<MockAllocator>(nullptr, &arrayInfo, &stateControl);
    NiceMock<MockJournalManager>* journal = new NiceMock<MockJournalManager>(&arrayInfo, &stateControl);

    Metadata meta(&arrayInfo, mapper, allocator, journal);

    EXPECT_CALL(*mapper, Dispose).Times(1);
    EXPECT_CALL(*allocator, Dispose).Times(1);
    EXPECT_CALL(*journal, Dispose).Times(1);

    // When
    meta.Dispose();

    // Then
}

TEST(Metadata, Shutdown_testIfAllComponentsAreDisposed)
{
    // Given
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIStateControl> stateControl;
    NiceMock<MockMapper>* mapper = new NiceMock<MockMapper>(&arrayInfo, &stateControl);
    NiceMock<MockAllocator>* allocator = new NiceMock<MockAllocator>(nullptr, &arrayInfo, &stateControl);
    NiceMock<MockJournalManager>* journal = new NiceMock<MockJournalManager>(&arrayInfo, &stateControl);

    Metadata meta(&arrayInfo, mapper, allocator, journal);

    // Then
    EXPECT_CALL(*mapper, Shutdown);
    EXPECT_CALL(*allocator, Shutdown);
    EXPECT_CALL(*journal, Shutdown);

    // When
    meta.Shutdown();
}
} // namespace pos
