#pragma once
#include <pp_common/_.hpp>

class xDR_DeviceManager final {
public:
    bool Init(size_t MaxDeviceCount);
    void Clean();

    uint64_t AddDeviceHandle(const xTcpServiceClientConnectionHandle & Handle);
    void     ReleaseDeviceHandle(uint64_t DeviceHandleId);

private:
    xel::xIndexedStorage<xTcpServiceClientConnectionHandle> DeviceHandleIdManager;
};
