#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_mgmt/array_manager.h"

namespace pos
{
class MockArrayManager : public ArrayManager
{
public:
    using ArrayManager::ArrayManager;
    MOCK_METHOD(int, Create, (string name, DeviceSet<string> devs, string raidtype), (override));
    MOCK_METHOD(int, Delete, (string name), (override));
    MOCK_METHOD(int, Mount, (string name), (override));
    MOCK_METHOD(int, Unmount, (string name), (override));
    MOCK_METHOD(int, AddDevice, (string name, string dev), (override));
    MOCK_METHOD(int, RemoveDevice, (string name, string dev), (override));
    MOCK_METHOD(int, DeviceDetached, (UblockSharedPtr dev), (override));
    MOCK_METHOD(void, DeviceAttached, (UblockSharedPtr dev), (override));
    MOCK_METHOD(int, PrepareRebuild, (string name, bool& resume), (override));
    MOCK_METHOD(void, RebuildDone, (string name), (override));
    MOCK_METHOD(int, Load, (list<string> & failedArrayList), (override));
    MOCK_METHOD(int, ResetMbr, (), (override));
};

} // namespace pos
