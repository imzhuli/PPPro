#include "./_global.hpp"

xNetAddress DeviceAddress4;
xNetAddress DeviceAddress6;
xNetAddress ProxyAddress4;
xNetAddress ProxyAddress6;

xNetAddress ExportDeviceAddress4;
xNetAddress ExportDeviceAddress6;
xNetAddress ExportProxyAddress4;
xNetAddress ExportProxyAddress6;

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListDownloadAddress;

xServerIdClient                          ServerIdClient;
xRelayInfoDispatcherServerInfoDownloader RIDDownloader;
xRelayInfoReporter                       RIReporter;
xDeviceStateRelayServerListDownloader    DSRDownloader;
xClientPoolWrapper                       DeviceReporter;

xTcpService DeviceService4;
xTcpService DeviceService6;
xTcpService ProxyService4;
xTcpService ProxyService6;

xRD_LocalAudit LocalAudit;

void xRD_LocalAudit::ResetPeriodicalValues() {

    Reset(NewRelayConnections);
    Reset(DeviceClosedConnections);
    Reset(ProxyClosedConnections);

    //
}

#define O(x) OS << #x << "=" << (x) << ' '

std::string xRD_LocalAudit::ToString() const {

    auto OS = std::ostringstream();

    O(TotalCtrlConnections);
    O(TotalDataConnections);
    O(TotalDeviceEnabled);
    O(TotalRelayConnections);

    O(NewRelayConnections);
    O(DeviceClosedConnections);
    O(ProxyClosedConnections);

    O(TotalEnabledTcp4);
    O(TotalEnabledTcp6);
    O(TotalEnabledUdp4);
    O(TotalEnabledUdp6);

    return OS.str();
}

#undef O

void AuditIncDeviceFlag(xDeviceFlag Flags) {
    if (Flags & DF_ENABLE_TCP4) {
        ++LocalAudit.TotalEnabledTcp4;
    }
    if (Flags & DF_ENABLE_TCP6) {
        ++LocalAudit.TotalEnabledTcp6;
    }
    if (Flags & DF_ENABLE_UDP4) {
        ++LocalAudit.TotalEnabledUdp4;
    }
    if (Flags & DF_ENABLE_UDP6) {
        ++LocalAudit.TotalEnabledUdp6;
    }
}

void AuditDecDeviceFlag(xDeviceFlag Flags) {
    if (Flags & DF_ENABLE_TCP4) {
        --LocalAudit.TotalEnabledTcp4;
    }
    if (Flags & DF_ENABLE_TCP6) {
        --LocalAudit.TotalEnabledTcp6;
    }
    if (Flags & DF_ENABLE_UDP4) {
        --LocalAudit.TotalEnabledUdp4;
    }
    if (Flags & DF_ENABLE_UDP6) {
        --LocalAudit.TotalEnabledUdp6;
    }
}
