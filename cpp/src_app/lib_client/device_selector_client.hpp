#pragma once
#include "../lib_server_list/device_selector_dispatcher_list_downloader.hpp"
#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>

struct xDeviceSelectorOptions {
    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
    bool       RequireIpv6;
    bool       RequireUdp;
    bool       RequireRemoteDns;

    std::string OptionEx;
};

struct xDeviceSelectorResult {
    uint64_t DeviceRelayServerRuntimeId;
    uint64_t DeviceRelaySideId;

    operator bool() { return DeviceRelayServerRuntimeId && DeviceRelaySideId; }
};

class xDeviceSelectorClient {
private:
    struct xRequestContext : xListNode {
        uint64_t RequestId;
        uint64_t StartTimestampMS;
        // source
        uint64_t SourceRequestId;
    };

public:
    bool Init(xIoContext * ICP, const xel::xNetAddress & ServerListAddress);
    void Tick(uint64_t NowMS);
    void Clean();

    bool HasValidBackend() { return ACCConnections; }
    bool Request(uint64_t SourceRequestId, const xDeviceSelectorOptions & Ops);

    // callback types
    using xOnEnabled                    = std::function<void()>;
    using xOnDisabled                   = std::function<void()>;
    using xOnDeviceSelectResultCallback = std::function<void(uint64_t RequestContextId, const xDeviceSelectorResult & Result)>;

    // callbacks
    xOnEnabled                    OnEnabled                    = Noop<>;
    xOnDisabled                   OnDisabled                   = Noop<>;
    xOnDeviceSelectResultCallback OnDeviceSelectResultCallback = Noop<>;

private:
    xTicker                                       T;
    xDeviceSelectorDispatcherServerListDownloader DSD;
    xClientPoolWrapper                            ACC;
    xIndexedStorage<xRequestContext>              RequestPool;
    xList<xRequestContext>                        RequestQueue;

    size_t ACCConnections = 0;
};