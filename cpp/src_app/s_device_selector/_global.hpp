#pragma once
#include "../lib_server_list/device_selector_dispatcher_list_downloader.hpp"
#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_utils/all.hpp"
#include "./device_context_manager.hpp"
#include "./device_selector.hpp"

#include <pp_common/_.hpp>

extern xNetAddress ServerListRegisterAddress;
extern xNetAddress ServerListDownloadAddress;

extern xDS_DeviceContextManager                      DeviceContextManager;
extern xDS_DeviceSelectorServiceProvider             DeviceSelectorService;
extern xDeviceStateRelayServerListDownloader         DSRDownloader;
extern xDeviceSelectorDispatcherServerListDownloader DSDDownloader;

extern xClientPoolWrapper DeviceObserver;

extern struct xDS_LocalAudit {
    uint64_t DurationMS = 0;

    size_t NewDeviceCount      = 0;
    size_t ReplacedDeviceCount = 0;
    size_t RemovedDeviceCount  = 0;
    size_t TimeoutDeviceCount  = 0;

    size_t TotalDeviceCount   = 0;
    size_t EnabledDeviceCount = 0;

    void        ResetPeriodCount();
    std::string ToString() const;
} LocalAudit;
