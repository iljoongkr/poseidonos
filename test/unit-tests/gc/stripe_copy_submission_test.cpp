#include "src/gc/stripe_copy_submission.h"

#include <gtest/gtest.h>
#include <src/include/address_type.h>
#include <src/include/partition_type.h>
#include <src/include/smart_ptr_type.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/copier_meta_mock.h>
#include <test/unit-tests/gc/stripe_copy_submission_mock.h>
#include <test/unit-tests/gc/victim_stripe_mock.h>
#include <test/unit-tests/lib/bitmap_mock.h>
#include <test/unit-tests/spdk_wrapper/free_buffer_pool_mock.h>
#include <test/unit-tests/cpu_affinity/affinity_manager_mock.h>
#include <test/unit-tests/utils/mock_builder.h>
#include <test/unit-tests/event_scheduler/event_scheduler_mock.h>
#include <test/unit-tests/gc/stripe_copier_mock.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;

namespace pos
{
static const uint32_t GC_BUFFER_COUNT = 512;
static const uint32_t GC_CONCURRENT_COUNT = 16;
static const uint32_t GC_VICTIM_SEGMENT_COUNT = 2;

class StripeCopySubmissionTestFixture : public ::testing::Test
{
public:
    StripeCopySubmissionTestFixture(void)
    : array(nullptr),
      meta(nullptr),
      inUseBitmap(nullptr),
      victimStripes(nullptr),
      gcBufferPool(nullptr)
    {
    }

    virtual ~StripeCopySubmissionTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        victimId = 0;
        baseStripeId = 0;
        copyIndex = 0;
        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        victimStripes = new std::vector<std::vector<VictimStripe*>>;
        victimStripes->resize(GC_VICTIM_SEGMENT_COUNT);
        for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
        {
            for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
            {
                (*victimStripes)[stripeIndex].push_back(new NiceMock<MockVictimStripe>(array, nullptr, nullptr, nullptr, nullptr));
            }
        }
        gcBufferPool = new std::vector<FreeBufferPool*>;
        affinityManager = new NiceMock<MockAffinityManager>(BuildDefaultAffinityManagerMock());
        for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
        {
            gcBufferPool->push_back(new NiceMock<MockFreeBufferPool>(0, 0, affinityManager));
        }

        meta = new NiceMock<MockCopierMeta>(array, udSize, inUseBitmap, nullptr, victimStripes, gcBufferPool);
    }

    virtual void
    TearDown(void)
    {
        delete stripeCopySubmission;
        delete array;
        delete meta;
        delete affinityManager;
    }

protected:
    StripeCopySubmission* stripeCopySubmission;

    SegmentId victimId;
    StripeId baseStripeId;
    uint32_t copyIndex;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockCopierMeta>* meta;
    NiceMock<MockBitMapMutex>* inUseBitmap;

    std::vector<std::vector<VictimStripe*>>* victimStripes;
    std::vector<FreeBufferPool*>* gcBufferPool;
    NiceMock<MockAffinityManager>* affinityManager;

    EventSmartPtr mockCopyEvent;
    EventSmartPtr mockStripeCopier;
    NiceMock<MockEventScheduler>* eventScheduler;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/* not interesting */,
    .blksPerChunk = 64,
    .blksPerStripe = 2048,
    .chunksPerStripe = 32,
    .stripesPerSegment = 1024,
    .totalStripes = 32,
    .totalSegments = 32768,
    };
};

TEST_F(StripeCopySubmissionTestFixture, Execute_testIfExecuteFailsWhenMetaModuleIsNotReadyToCopy)
{
    eventScheduler = new NiceMock<MockEventScheduler>;
    mockStripeCopier = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        nullptr, nullptr, eventScheduler);
    stripeCopySubmission = new StripeCopySubmission(0, meta, 0, mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetStripePerSegment).WillOnce(Return(0));
    EXPECT_CALL(*meta, IsReadytoCopy(copyIndex)).WillOnce(Return(false));

    EXPECT_TRUE(stripeCopySubmission->Execute() == false);

    delete eventScheduler;
    mockStripeCopier == nullptr;
}

TEST_F(StripeCopySubmissionTestFixture, Execute_testStripeCopySubmissionFaliWhenNotReadytoCopy)
{
    eventScheduler = new NiceMock<MockEventScheduler>;
    mockStripeCopier = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        nullptr, nullptr, eventScheduler);
    stripeCopySubmission = new StripeCopySubmission(0, meta, 0, mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetStripePerSegment).WillRepeatedly(Return(10));
    for (uint32_t index = 0; index < 10; index++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(copyIndex, index)).WillOnce(Return(&victimStripe));
    }
    EXPECT_CALL(victimStripe, LoadValidBlock).Times(10);
    EXPECT_CALL(*meta, IsReadytoCopy(copyIndex)).WillOnce(Return(false));

    EXPECT_TRUE(stripeCopySubmission->Execute() == false);

    delete eventScheduler;
    mockStripeCopier == nullptr;
}

TEST_F(StripeCopySubmissionTestFixture, Execute_teststripeCopySubmissionExecuteWhenReadyToCopyThenEnqueueEvent)
{
    eventScheduler = new NiceMock<MockEventScheduler>;
    mockStripeCopier = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        nullptr, nullptr, eventScheduler);
    stripeCopySubmission = new StripeCopySubmission(0, meta, 0, mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetStripePerSegment).WillRepeatedly(Return(10));
    for (uint32_t index = 0; index < 10; index++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(copyIndex, index)).WillOnce(Return(&victimStripe));
    }
    EXPECT_CALL(victimStripe, LoadValidBlock).Times(10);
    EXPECT_CALL(*meta, IsReadytoCopy(copyIndex)).WillOnce(Return(true));
    EXPECT_CALL(*eventScheduler, EnqueueEvent(mockStripeCopier)).Times(GC_CONCURRENT_COUNT);

    EXPECT_TRUE(stripeCopySubmission->Execute() == true);

    delete eventScheduler;
    mockStripeCopier == nullptr;
}
} // namespace pos
