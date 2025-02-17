#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/mapper/mapper.h"

namespace pos
{
class MockVolumeMountState : public VolumeMountState
{
public:
    using VolumeMountState::VolumeMountState;
};

class MockMapper : public Mapper
{
public:
    using Mapper::Mapper;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(IVSAMap*, GetIVSAMap, (), (override));
    MOCK_METHOD(IStripeMap*, GetIStripeMap, (), (override));
    MOCK_METHOD(IReverseMap*, GetIReverseMap, (), (override));
    MOCK_METHOD(IMapFlush*, GetIMapFlush, (), (override));
    MOCK_METHOD(IMapperWbt*, GetIMapperWbt, (), (override));
    MOCK_METHOD(IMapperVolumeEventHandler*, GetVolumeEventHandler, (), (override));
    MOCK_METHOD(bool, VolumeCreated, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeMounted, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeLoaded, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (int volId, bool flushMapRequired), (override));
    MOCK_METHOD(int, PrepareVolumeDelete, (int volId), (override));
    MOCK_METHOD(int, DeleteVolumeMap, (int volumeId), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList), (override));
    MOCK_METHOD(int, GetVSAs, (int volId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray), (override));
    MOCK_METHOD(int, SetVSAs, (int volId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(VirtualBlkAddr, GetRandomVSA, (BlkAddr rba), (override));
    MOCK_METHOD(int64_t, GetNumUsedBlks, (int volId), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVSAInternal, (int volId, BlkAddr rba, int& retry), (override));
    MOCK_METHOD(int, SetVSAsInternal, (int volId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVSAWithSyncOpen, (int volId, BlkAddr rba), (override));
    MOCK_METHOD(int, SetVSAsWithSyncOpen, (int volId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(MpageList, GetDirtyVsaMapPages, (int volId, BlkAddr startRba, uint64_t numBlks), (override));
    MOCK_METHOD(int, EnableInternalAccess, (int volId), (override));
    MOCK_METHOD(int, FlushDirtyMpages, (int mapId, EventSmartPtr callback, MpageList dirtyPages), (override));
    MOCK_METHOD(int, StoreAll, (), (override));
};

} // namespace pos
