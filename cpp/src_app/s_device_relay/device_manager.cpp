#include "./device_manager.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/device_state.hpp>

bool xDeviceManager::Init(size_t MaxDevice) {
    if (!DevicePool.Init(MaxDevice)) {
        return false;
    }
    auto DPC = xScopeCleaner(DevicePool);

    DPC.Dismiss();
    return true;
}

void xDeviceManager::Clean() {
    // build kill list

    // destroy any device context in kill list
    while (auto DC = DeviceKillList.PopHead()) {
        DestroyDevice(DC);
    }
    auto DPC = xScopeCleaner(DevicePool);
}

void xDeviceManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
}

auto xDeviceManager::CreateDevice() -> xDevice * {
    auto ContextId = DevicePool.Acquire();
    if (!ContextId) {
        return nullptr;
    }
    auto & ContextRef          = DevicePool[ContextId];
    ContextRef.DeviceRuntimeId = ContextId;
    return &ContextRef;
}

void xDeviceManager::DestroyDevice(xDevice * DC) {
    DevicePool.Release(DC->DeviceRuntimeId);
}

void xDeviceManager::ReportDeviceState(xDevice * Device, bool Offline) {
    X_DEBUG_PRINTF(" DeviceRuntimeId: %" PRIx64 "", Device->DeviceRuntimeId);
    assert(Device);
    auto A = xPP_DeviceInfoUpdate();

    A.RelayServerRuntimeId = ServerRuntimeId;
    A.RelaySideDeviceKey   = Device->DeviceRuntimeId;

    A.PrimaryIpv4Address = Device->PrimaryIpv4;
    A.PrimaryIpv6Address = Device->PrimaryIpv6;

    A.CountryId = Device->GeoInfo.CountryId;
    A.StateId   = Device->GeoInfo.StateId;
    A.CityId    = Device->GeoInfo.CityId;

    A.DeviceUuid = Device->DeviceLocalIdString;
    A.IsOffline  = Offline;

    DeviceReporter.PostMessage(Cmd_DSR_DS_DeviceUpdate, 0, A);
}

//
