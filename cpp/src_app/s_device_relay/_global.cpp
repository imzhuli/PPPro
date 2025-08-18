#include "./_global.hpp"

xNetAddress BindAddress4;
xNetAddress BindAddress6;
xNetAddress ProxyAddress4;
xNetAddress ProxyAddress6;

xNetAddress ExportAddress4;
xNetAddress ExportAddress6;
xNetAddress ExportProxyAddress4;
xNetAddress ExportProxyAddress6;

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListDownloadAddress;

xServerIdClient                          ServerIdClient;
xRelayInfoDispatcherServerInfoDownloader RIDDownloader;
xRelayInfoReporter                       RIReporter;
xDeviceStateRelayServerListDownloader    DSRDownloader;
xClientPoolWrapper                       DeviceReporter;

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

    return OS.str();
}

#undef O