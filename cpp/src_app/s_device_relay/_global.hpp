#pragma once
#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_server_list/relay_info_dispatcher_server_info_downloader.hpp"
#include "../lib_utils/all.hpp"
#include "./relay_server_info_reporter.hpp"

#include <pp_common/_.hpp>

static constexpr const size32_t MaxDeviceCount          = 10'0000;
static constexpr const size32_t MaxProxyCount           = 3000;
static constexpr const size32_t MaxRelayConnectionCount = 30'0000;

extern xNetAddress BindAddress4;
extern xNetAddress BindAddress6;
extern xNetAddress ProxyAddress4;
extern xNetAddress ProxyAddress6;

extern xNetAddress ExportAddress4;
extern xNetAddress ExportAddress6;
extern xNetAddress ExportProxyAddress4;
extern xNetAddress ExportProxyAddress6;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListDownloadAddress;

extern xServerIdClient                          ServerIdClient;
extern xRelayInfoDispatcherServerInfoDownloader RIDDownloader;
extern xRelayInfoReporter                       RIReporter;
extern xDeviceStateRelayServerListDownloader    DSRDownloader;
extern xClientPoolWrapper                       DeviceReporter;

struct xRD_LocalAudit {
    uint64_t TotalCtrlConnections;
    uint64_t TotalDataConnections;
    uint64_t TotalDeviceEnabled;
    uint64_t TotalRelayConnections;

    // new de
    uint64_t NewRelayConnections;
    uint64_t NewEnabledDevices;
    uint64_t DeviceClosedConnections;
    uint64_t ProxyClosedConnections;

    uint64_t MissingLostDeviceCount;

    void ResetPeriodicalValues();
    auto ToString() const -> std::string;
};
extern xRD_LocalAudit LocalAudit;
