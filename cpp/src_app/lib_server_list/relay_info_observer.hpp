#pragma once
#include "../lib_utils/all.hpp"
#include "./relay_info_dispatcher_server_info_downloader.hpp"

#include <pp_common/_.hpp>
#include <pp_protocol/internal/all.hpp>

struct xRIO_RelayServerInfoContext final {
    xRelayServerInfoBase ServerInfo;
    xel::xVariable mutable MutableUserContext = {};

    std::string ToString() const { return ServerInfo.ToString(); }
};

class xRelayInfoObserver final : xel::xNonCopyable {

    static constexpr const uint64_t RELAY_INFO_TIMEOUT_MS = 105'000;

    struct xManagedRelayServerInfo : xListNode {
        xRIO_RelayServerInfoContext Context;
        uint64_t                    LastKeepAliveTimestampMS = 0;
    };
    using xRelayInfoTimeoutList = xList<xManagedRelayServerInfo>;

public:
    using xOnNewDeviceRelayInfoCallback      = std::function<void(const xRIO_RelayServerInfoContext & Info)>;
    using xOnNewDeviceRelayInfoErrorCallback = std::function<void(const xRIO_RelayServerInfoContext & Info)>;
    using xOnRemoveDeviceRelayInfoCallback   = std::function<void(const xRIO_RelayServerInfoContext & Info)>;

public:
    bool Init(xIoContext * ICP, const xNetAddress & ServerListDownloadAddress);
    void Clean();
    void Tick(uint64_t NowMS);

    xOnNewDeviceRelayInfoCallback    OnNewDeviceRelayInfoCallback    = NOOP<const xRIO_RelayServerInfoContext &>;
    xOnRemoveDeviceRelayInfoCallback OnRemoveDeviceRelayInfoCallback = NOOP<const xRIO_RelayServerInfoContext &>;

private:
    bool OnRelayInfoDispatcherPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    bool OnBroadcastRelayInfo(ubyte * PayloadPtr, size_t PayloadSize);
    bool OnBroadcastRelayOffline(ubyte * PayloadPtr, size_t PayloadSize);

    void InsertOrKeepAliveDeviceRelayInfo(const xRelayServerInfoBase & Info);
    void RemoveDeviceRelayInfo(xManagedRelayServerInfo * RSI);

private:
    xTicker                                  Ticker;
    xClientWrapper                           RelayInfoDispatcherClient;
    xRelayInfoDispatcherServerInfoDownloader DispatcherServerInfoDownloader;

    std::map<uint64_t, uint64_t>             DeviceRelayServerIdLocalMap;
    xRelayInfoTimeoutList                    RelayInfoTimeoutList;
    xIndexedStorage<xManagedRelayServerInfo> DeviceRelayInfoPool;
};