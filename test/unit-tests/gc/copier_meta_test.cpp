#include "src/gc/copier_meta.h"

#include <gtest/gtest.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/gc_stripe_manager_mock.h>
#include <test/unit-tests/gc/victim_stripe_mock.h>
#include <test/unit-tests/lib/bitmap_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/mapper/i_reversemap_mock.h>
#include <test/unit-tests/mapper/reversemap/reverse_map_mock.h>
#include <test/unit-tests/spdk_wrapper/free_buffer_pool_mock.h>
#include <test/unit-tests/cpu_affinity/affinity_manager_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;

namespace pos
{
class CopierMetaTestFixture : public ::testing::Test
{
public:
    CopierMetaTestFixture(void)
    : array(nullptr),
      inUseBitmap(nullptr),
      gcStripeManager(nullptr),
      reverseMapPack(nullptr),
      gcWriteBufferPool(nullptr),
      volumeEventPublisher(nullptr),
      victimStripes(nullptr),
      gcBufferPool(nullptr)
    {
    }

    virtual ~CopierMetaTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));
        EXPECT_CALL(*array, GetName).WillRepeatedly(Return("POSArray"));

        affinityManager = new NiceMock<MockAffinityManager>(BuildDefaultAffinityManagerMock());
        gcWriteBufferPool = new NiceMock<MockFreeBufferPool>(0, 0, affinityManager);
        volumeEventPublisher = new NiceMock<MockVolumeEventPublisher>();
        gcStripeManager = new NiceMock<MockGcStripeManager>(array, gcWriteBufferPool, volumeEventPublisher);
        inUseBitmap = new NiceMock<MockBitMapMutex>(2);

        victimStripes = new std::vector<std::vector<VictimStripe*>>;
        victimStripes->resize(GC_VICTIM_SEGMENT_COUNT);
        for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
        {
            for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
            {
                reverseMapPack = new NiceMock<MockReverseMapPack>;
                (*victimStripes)[stripeIndex].push_back(new NiceMock<MockVictimStripe>(array, reverseMapPack, nullptr, nullptr, nullptr));
            }
        }

        gcBufferPool = new std::vector<FreeBufferPool*>;
        for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
        {
            gcBufferPool->push_back(new NiceMock<MockFreeBufferPool>(0, 0, affinityManager));
        }

        copierMeta = new CopierMeta(array, udSize, inUseBitmap, gcStripeManager, victimStripes, gcBufferPool);
    }

    virtual void
    TearDown(void)
    {
        delete copierMeta;
        delete array;
        delete affinityManager;
    }

protected:
    CopierMeta* copierMeta;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockBitMapMutex>* inUseBitmap;
    NiceMock<MockAffinityManager>* affinityManager;
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockReverseMapPack>* reverseMapPack;
    NiceMock<MockFreeBufferPool>* gcWriteBufferPool;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;

    std::vector<std::vector<VictimStripe*>>* victimStripes;

    std::vector<FreeBufferPool*>* gcBufferPool;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0,/* no interesting */
    .blksPerChunk = 64,
    .blksPerStripe = 2048,
    .chunksPerStripe = 32,
    .stripesPerSegment = 1024,
    .totalStripes = 32,
    .totalSegments = 32768
    };
    uint32_t GC_BUFFER_COUNT = 512;
    uint32_t GC_CONCURRENT_COUNT = 16;
    uint32_t GC_VICTIM_SEGMENT_COUNT = 2;
};

TEST_F(CopierMetaTestFixture, GetBuffer_testIfBufferIsReturnedFromGcBufferPoolBasedOnStripeId)
{
    // 
    uint32_t testArgs[] = {1, GC_BUFFER_COUNT, GC_BUFFER_COUNT + 1, GC_BUFFER_COUNT * 2 + 1};
    for (uint32_t stripeId : testArgs)
    {
        EXPECT_CALL(*dynamic_cast<MockFreeBufferPool*>((*gcBufferPool)[stripeId % GC_BUFFER_COUNT]), GetBuffer).WillOnce(Return((void*)0x20000000));
        void* bufferAddr;
        bufferAddr = copierMeta->GetBuffer(stripeId);
        EXPECT_TRUE((void*)0x20000000 == bufferAddr);
    }
}

TEST_F(CopierMetaTestFixture, ReturnBuffer_testIfBufferIsReturnedBasedOnStripeId)
{
    uint32_t testArgs[] = {1, GC_BUFFER_COUNT, GC_BUFFER_COUNT + 1, GC_BUFFER_COUNT * 2 + 1};
    for (uint32_t stripeId : testArgs)
    {
        uint32_t bufferAddr = 0x20000000;
        void* ptr = &bufferAddr;
        EXPECT_CALL(*dynamic_cast<MockFreeBufferPool*>((*gcBufferPool)[stripeId % GC_BUFFER_COUNT]), ReturnBuffer(ptr)).Times(1);
        copierMeta->ReturnBuffer(stripeId, ptr);
    }
}

TEST_F(CopierMetaTestFixture, SetStartCopyStripes_testEnoughCalledSetStartCopyStripesWithoutCopyBlock)
{
    // when enough called SetStartCopyStripes without copy block
    for (uint32_t index = 0; index < STRIPES_PER_SEGMENT; index++)
    {
        // not enough started copy stripes
        EXPECT_TRUE(copierMeta->IsCopyDone() == false);
        copierMeta->SetStartCopyStripes();
    }
    EXPECT_CALL(*inUseBitmap, IsSetBit(_)).WillOnce(Return(true));

    //then return copy done
    EXPECT_TRUE(copierMeta->IsCopyDone() == true);

}

TEST_F(CopierMetaTestFixture, SetStartCopyBlks_testAddStartCopyBlocksWithTestBlocks)
{
    uint32_t testBlocks = 10;
    for (uint32_t count = 1; count <= 10; count++)
    {
        copierMeta->SetStartCopyBlks(testBlocks);
        EXPECT_TRUE(copierMeta->GetStartCopyBlks() == (testBlocks * count));
    }
}

TEST_F(CopierMetaTestFixture, SetDoneCopyBlks_testAddCopiedBlocksWithTestBlocks)
{
    uint32_t testBlocks = 10;
    for (uint32_t count = 1; count <= 10; count++)
    {
        copierMeta->SetDoneCopyBlks(testBlocks);
        EXPECT_TRUE(copierMeta->GetDoneCopyBlks() == (testBlocks * count));
    }
}

TEST_F(CopierMetaTestFixture, InitProgressCount_testInitProgressArgumentCount)
{
    // given add progress count
    uint32_t testBlocks = 10;
    copierMeta->SetStartCopyBlks(testBlocks);
    copierMeta->SetDoneCopyBlks(testBlocks);
    EXPECT_TRUE(copierMeta->GetStartCopyBlks() == testBlocks);
    EXPECT_TRUE(copierMeta->GetDoneCopyBlks() == testBlocks);

    // when init progress count
    copierMeta->InitProgressCount();

    // then return initial progress count
    EXPECT_TRUE(copierMeta->GetDoneCopyBlks() == 0);
    EXPECT_TRUE(copierMeta->GetStartCopyBlks() == 0);
}

TEST_F(CopierMetaTestFixture, SetInUseBitmap_testReturnVictimSegmentIndexInCopierMetaAndAddVictimSegmentIndex)
{
    // initial index is 0 in copier meta, return index and increase index
    EXPECT_CALL(*inUseBitmap, SetBit(0)).Times(1);
    EXPECT_TRUE(copierMeta->SetInUseBitmap() == 0);

    // index is 1 in copier meta, return index
    EXPECT_CALL(*inUseBitmap, SetBit(1)).Times(1);
    EXPECT_TRUE(copierMeta->SetInUseBitmap() == 1);
}

TEST_F(CopierMetaTestFixture, IsSynchronized_testCheckClearedInUseBitWithIsSynchronized)
{
    // return IsSetBit true in InUseBitmap isSynchronized return false
    EXPECT_CALL(*inUseBitmap, IsSetBit(0)).WillOnce(Return(true));
    EXPECT_TRUE(copierMeta->IsSynchronized() == false);

    // return IsSetBit false in InUseBitmap isSynchronized return true
    EXPECT_CALL(*inUseBitmap, IsSetBit(0)).WillOnce(Return(false));
    EXPECT_TRUE(copierMeta->IsSynchronized() == true);
}

TEST_F(CopierMetaTestFixture, IsAllVictimSegmentCopyDone_testAllVictimSegmentCopyDone)
{
    // when victim segment copy done, clear InUseBitmap
    for (uint32_t index = 0; index < GC_VICTIM_SEGMENT_COUNT; index++)
    {
        EXPECT_CALL(*inUseBitmap, IsSetBit(index)).WillOnce(Return(false));
    }

    // then return true
    EXPECT_TRUE(copierMeta->IsAllVictimSegmentCopyDone() == true);
}

TEST_F(CopierMetaTestFixture, IsAllVictimSegmentCopyDone_testSomeVictimSegmentAreNotCopyDone)
{
    // when some victimSemgnet are not copy done
    EXPECT_CALL(*inUseBitmap, IsSetBit(0)).WillOnce(Return(true));
    // then return false
    EXPECT_TRUE(copierMeta->IsAllVictimSegmentCopyDone() == false);

    // when some victimSemgnet are not copy done
    EXPECT_CALL(*inUseBitmap, IsSetBit(0)).WillOnce(Return(false));
    EXPECT_CALL(*inUseBitmap, IsSetBit(1)).WillOnce(Return(true));
    // then return false
    EXPECT_TRUE(copierMeta->IsAllVictimSegmentCopyDone() == false);
}

TEST_F(CopierMetaTestFixture, IsCopyDone_testIsCopyDoneWithStartCopiedStripesAndCopiedBlocks)
{
    uint32_t testBlock = 10;
    // given enough called SetStartCopyStripes and set started copy blocks
    for (uint32_t index = 0; index < STRIPES_PER_SEGMENT; index++)
    {
        // not enough set start copy stripes
        EXPECT_TRUE(copierMeta->IsCopyDone() == false);
        copierMeta->SetStartCopyStripes();
        copierMeta->SetStartCopyBlks(testBlock);
    }

    // given added copied block count
    for (uint32_t index = 0; index < STRIPES_PER_SEGMENT; index++)
    {
        // when Check copy done then return false
        EXPECT_TRUE(copierMeta->IsCopyDone() == false);
        copierMeta->SetDoneCopyBlks(testBlock);
    }
    EXPECT_CALL(*inUseBitmap, IsSetBit(_)).WillOnce(Return(true));

    //then return copy done
    EXPECT_TRUE(copierMeta->IsCopyDone() == true);
}

TEST_F(CopierMetaTestFixture, IsReadytoCopy_testCheckReadytoCopyWithCopySlotIndex)
{
    uint32_t copySlotIndex = 0;
    // before start to copy, check ready to copy with copy slot index that controlled by copierMeta
    // then return true when success locking copy slot
    EXPECT_TRUE(copierMeta->IsReadytoCopy(copySlotIndex) == true);
    // then return false because already locked
    EXPECT_TRUE(copierMeta->IsReadytoCopy(copySlotIndex) == false);

    copySlotIndex++;
    copySlotIndex = copySlotIndex % GC_VICTIM_SEGMENT_COUNT;
    // then return false because copy slot index did not change
    EXPECT_TRUE(copierMeta->IsReadytoCopy(1) == false);

    for (uint32_t index = 0; index < STRIPES_PER_SEGMENT; index++)
    {
        copierMeta->SetStartCopyStripes();
    }
    EXPECT_CALL(*inUseBitmap, IsSetBit(_)).WillOnce(Return(true));
    EXPECT_TRUE(copierMeta->IsCopyDone() == true);

    // then return true
    EXPECT_TRUE(copierMeta->IsReadytoCopy(1) == true);

    for (uint32_t index = 0; index < STRIPES_PER_SEGMENT; index++)
    {
        copierMeta->SetStartCopyStripes();
    }
    EXPECT_CALL(*inUseBitmap, IsSetBit(_)).WillOnce(Return(true));
    EXPECT_TRUE(copierMeta->IsCopyDone() == true);
    copySlotIndex++;
    copySlotIndex = copySlotIndex % GC_VICTIM_SEGMENT_COUNT;

    EXPECT_TRUE(copierMeta->IsReadytoCopy(0) == true);
}

TEST_F(CopierMetaTestFixture, GetStripePerSegment_testIfStripePerSegmentIsRetrievedFromPartitionLogicalSize)
{
    // given partition logical size when create copier meta 
    // when GetStripePerSegment
    // then return stripesPerSegment in partitionLogicalSize
    EXPECT_TRUE(copierMeta->GetStripePerSegment() == partitionLogicalSize.stripesPerSegment);
}

TEST_F(CopierMetaTestFixture, GetBlksPerStripe_testIfBlocksperStripeIsRetrievedFromPartitionLogicalSize)
{
    // given partition logical size when create copier meta 
    // when GetBlksPerStripe
    // then return blksPerStripe in partitionLogicalSize
    EXPECT_TRUE(copierMeta->GetBlksPerStripe() == partitionLogicalSize.blksPerStripe);
}

TEST_F(CopierMetaTestFixture, GetVictimStripe_testReturnVictimStripeFromCopierMeta)
{
    // given victimStripes when create copier meta 
    // when GetVictimStripe
    // then return victimStripe
    uint32_t segmentIndexArray[] = {0, GC_VICTIM_SEGMENT_COUNT - 1};
    uint32_t stripeOffsetArray[] = {0, 1, partitionLogicalSize.stripesPerSegment / 2,
                                partitionLogicalSize.stripesPerSegment - 1};
    for (uint32_t segmentIndex : segmentIndexArray)
    {
        for (uint32_t stripeOffset : stripeOffsetArray)
        {
            EXPECT_TRUE(copierMeta->GetVictimStripe(segmentIndex, stripeOffset)
                        == (*victimStripes)[segmentIndex][stripeOffset]);
        }
    }
}

TEST_F(CopierMetaTestFixture, GetGcStripeManager_testReturnGcStripeManagerFromCopierMeta)
{
    // given gcStripeManager when create copier meta 
    // when GetGcStripeManager
    // then return gcStripeManager
    EXPECT_TRUE(copierMeta->GetGcStripeManager() == gcStripeManager);
}

TEST_F(CopierMetaTestFixture, GetArrayName_testIfArrayNameRetrievedFromArrayInfo)
{
    // given array info when create copier meta 
    // when GetArrayname
    // then return array name in array info
    EXPECT_TRUE(copierMeta->GetArrayName() == "POSArray");
}

} // namespace pos
