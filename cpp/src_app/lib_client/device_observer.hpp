#pragma once
#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>
#include <unordered_map>

struct xDeviceInfoMin : xListNode {
    uint64_t LastActiveTimestampMS = 0;
    uint64_t RelayServerId         = 0;
    uint64_t DevcieConnectionId    = 0;
    struct {
        uint8_t V4 : 1  = 0;
        uint8_t V6 : 1  = 0;
        uint8_t Udp : 1 = 0;
    };
};

class xDeviceObserver {
public:
    bool Init(xel::xIoContext * ICP, const xel::xNetAddress & ServerListServiceAddress);
    void Clean();
    void Tick(uint64_t NowMS);

    using xOnDeviceOnline  = std::function<void(std::string && Uuid, const xDeviceInfoMin * InfoPtr)>;
    using xOnDeviceOffline = std::function<void(const std::string & Uuid)>;

    xOnDeviceOnline  OnDeviceOnline  = Noop<>;
    xOnDeviceOffline OnDeviceOffline = Noop<>;

private:
    xDeviceStateRelayServerListDownloader           DSRDownloader;
    xClientPoolWrapper                              DeviceObserver;
    std::unordered_map<std::string, xDeviceInfoMin> DeviceInfoMap;
};
