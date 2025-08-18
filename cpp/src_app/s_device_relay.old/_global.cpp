#include "./_global.hpp"

uint64_t   ServerRuntimeId = {};
xIoContext GlobalIoContext = {};

xNetAddress BindCtrlAddress;
xNetAddress BindDataAddress;
xNetAddress BindProxyAddress;

xNetAddress ExportCtrlAddress;
xNetAddress ExportDataAddress;
xNetAddress ExportProxyAddress;

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListDownloadAddress;

xRelayInfoDispatcherServerInfoDownloader RIDDownloader;
xDeviceStateRelayServerListDownloader    DSRDownloader;
xClientPoolWrapper                       DeviceReporter;

xRD_DeviceConnectionManager DeviceConnectionManager;
xDeviceManager              DeviceManager;
xRD_ProxyConnectionManager  ProxyConnectionManager;
xDeviceRelayService         DeviceRelayService;
xRD_RelayConnectionManager  RelayConnectionManager;
xRelayInfoReporter          RelayInfoReporter;
xServerIdClient             ServerIdClient;

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
