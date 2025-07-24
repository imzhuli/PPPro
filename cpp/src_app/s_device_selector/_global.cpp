#include "./_global.hpp"

#include <config/config.hpp>

xNetAddress BindAddress;
xNetAddress ServerListRegisterAddress;
xNetAddress ServerListDownloadAddress;

xDS_DeviceContextManager              DeviceContextManager;
xDS_DeviceSelectorService             DeviceSelectorService;
xDeviceStateRelayServerListDownloader DSRDownloader;
xClientPoolWrapper                    DeviceObserver;

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

    return OS.str();
}
