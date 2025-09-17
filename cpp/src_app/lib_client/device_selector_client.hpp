#pragma once
#include "../lib_server_list/device_selector_dispatcher_list_downloader.hpp"
#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>

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
    bool Request(const std::string_view & UserPassword, uint64_t SourceRequestId);

    // callback types
    using xOnEnabled                 = std::function<void()>;
    using xOnDisabled                = std::function<void()>;
    using xOnAuthCacheResultCallback = std::function<void(uint64_t RequestContextId, const xClientAuthResult * AuthResult)>;
    // callbacks
    xOnEnabled                 OnEnabled                 = Noop<>;
    xOnDisabled                OnDisabled                = Noop<>;
    xOnAuthCacheResultCallback OnAuthCacheResultCallback = Noop<>;

private:
    xTicker                                       T;
    xDeviceSelectorDispatcherServerListDownloader DSD;
    xClientPoolWrapper                            ACC;
    xIndexedStorage<xRequestContext>              RequestPool;
    xList<xRequestContext>                        RequestQueue;

    size_t ACCConnections = 0;
};