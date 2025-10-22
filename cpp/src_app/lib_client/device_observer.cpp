#include "./device_observer.hpp"

bool xDeviceObserver::Init(xel::xIoContext * ICP, const xel::xNetAddress & ServerListServiceAddress) {
    if (!DeviceObserver.Init(ICP, 1000)) {
        return false;
    }
    auto DOG = xScopeCleaner(DeviceObserver);

    if (!DSRDownloader.Init(ICP, ServerListServiceAddress)) {
        return false;
    }
    auto DSRDG = xScopeCleaner(DSRDownloader);

    DSRDownloader.OnUpdateDeviceStateRelayServerListCallback = [this](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & DSRInfo) {
        auto OSL = std::vector<xNetAddress>();
        for (auto & S : DSRInfo) {
            OSL.push_back(S.ObserverAddress);
        }
        DeviceObserver.UpdateServerList(OSL);
    };

    DOG.Dismiss();
    DSRDG.Dismiss();
    return true;
}

void xDeviceObserver::Clean() {
    Renew(DeviceInfoMap);
    DSRDownloader.Clean();
}

void xDeviceObserver::Tick(uint64_t NowMS) {
    DSRDownloader.Tick(NowMS);
}
