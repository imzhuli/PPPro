#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress ServerListRegisterAddress;
xNetAddress ServerListDownloadAddress;

xDS_DeviceContextManager                      DeviceContextManager;
xDS_DeviceSelectorServiceProvider             DeviceSelectorService;
xDeviceStateRelayServerListDownloader         DSRDownloader;
xDeviceSelectorDispatcherServerListDownloader DSDDownloader;
xClientPoolWrapper                            DeviceObserver;

xDS_LocalAudit LocalAudit;

void xDS_LocalAudit::ResetPeriodCount() {
    Reset(DurationMS);
    Reset(NewDeviceCount);
    Reset(ReplacedDeviceCount);
    Reset(RemovedDeviceCount);
    Reset(TimeoutDeviceCount);
}

#define O(x) OS << #x << "=" << (x) << ' '
std::string xDS_LocalAudit::ToString() const {
    auto OS = std::ostringstream();

    O(DurationMS);

    O(NewDeviceCount);
    O(ReplacedDeviceCount);
    O(RemovedDeviceCount);
    O(TimeoutDeviceCount);

    O(TotalDeviceCount);
    O(EnabledDeviceCount);

    return OS.str();
}
