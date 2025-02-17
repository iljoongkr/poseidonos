#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/gc_stripe_manager.h"

namespace pos
{
class MockGcStripeManager : public GcStripeManager
{
public:
    using GcStripeManager::GcStripeManager;
    MOCK_METHOD(bool, VolumeCreated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeLoaded, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUpdated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeMounted, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeDeleted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(GcAllocateBlks, AllocateWriteBufferBlks, (uint32_t volumeId, uint32_t numBlks), (override));
    MOCK_METHOD(void, SetFinished, (), (override));
    MOCK_METHOD(void, ReturnBuffer, (GcWriteBuffer* buffer), (override));
    MOCK_METHOD(GcWriteBuffer*, GetWriteBuffer, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, DecreaseRemainingAndCheckIsFull, (uint32_t volumeId, uint32_t cnt), (override));
    MOCK_METHOD(void, SetBlkInfo, (uint32_t volumeId, uint32_t offset, BlkInfo blkInfo), (override));
    MOCK_METHOD(std::vector<BlkInfo>*, GetBlkInfoList, (uint32_t volumeId), (override));
    MOCK_METHOD(void, SetFlushed, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, IsAllFinished, (), (override));
};

} // namespace pos
