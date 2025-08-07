#pragma once
#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_server_list/relay_info_dispatcher_server_info_downloader.hpp"
#include "../lib_utils/all.hpp"
#include "./device_manager.hpp"
#include "./relay_connection_manager.hpp"
#include "./relay_server_info_reporter.hpp"
#include "./relay_service.hpp"

#include <pp_common/_.hpp>

static constexpr const size32_t MaxDeviceCount          = 10'0000;
static constexpr const size32_t MaxProxyCount           = 3000;
static constexpr const size32_t MaxRelayConnectionCount = 30'0000;

extern uint64_t ServerRuntimeId;

extern xNetAddress BindCtrlAddress;
extern xNetAddress BindDataAddress;
extern xNetAddress BindProxyAddress;

extern xNetAddress ExportCtrlAddress;
extern xNetAddress ExportDataAddress;
extern xNetAddress ExportProxyAddress;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListDownloadAddress;
extern xNetAddress AuditDeviceAddress;

extern xRelayInfoDispatcherServerInfoDownloader RIDDownloader;
extern xDeviceStateRelayServerListDownloader    DSRDownloader;
extern xClientPoolWrapper                       DeviceReporter;

extern xRD_DeviceConnectionManager DeviceConnectionManager;
extern xDeviceManager              DeviceManager;
extern xRD_ProxyConnectionManager  ProxyConnectionManager;
extern xDeviceRelayService         DeviceRelayService;
extern xRD_RelayConnectionManager  RelayConnectionManager;
extern xRelayInfoReporter          RelayInfoReporter;
extern xServerIdClient             ServerIdClient;

extern struct xRD_LocalAudit {
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

} LocalAudit;
