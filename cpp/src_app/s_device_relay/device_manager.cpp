#include "./device_manager.hpp"

bool xDR_DeviceManager::Init(size_t MaxDeviceCount) {
    if (!DeviceHandleIdManager.Init(MaxDeviceCount)) {
        return false;
    }
    return true;
}

void xDR_DeviceManager::Clean() {
    DeviceHandleIdManager.Clean();
}

uint64_t xDR_DeviceManager::AddDeviceHandle(const xTcpServiceClientConnectionHandle & Handle) {
    return DeviceHandleIdManager.Acquire(Handle);
}

void xDR_DeviceManager::ReleaseDeviceHandle(uint64_t DeviceHandleId) {
    DeviceHandleIdManager.CheckAndRelease(DeviceHandleId);
}
