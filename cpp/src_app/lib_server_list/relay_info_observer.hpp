#pragma once
#include "../lib_utils/all.hpp"
#include "./relay_info_dispatcher_server_info_downloader.hpp"

#include <pp_common/_.hpp>
#include <pp_protocol/internal/all.hpp>

class xRelayInfoObserver final : xel::xNonCopyable {

    static constexpr const uint64_t RELAY_INFO_TIMEOUT_MS = 105'000;

    struct xRelayServerInfo : xListNode {
        xRelayServerInfoBase Info;
        uint64_t             LastKeepAliveTimestampMS = 0;
    };
    using xRelayInfoTimeoutList = xList<xRelayServerInfo>;

public:
    using xOnNewDeviceRelayInfoCallback      = std::function<void(const xRelayServerInfoBase & Info)>;
    using xOnNewDeviceRelayInfoErrorCallback = std::function<void(const xRelayServerInfoBase & Info)>;
    using xOnRemoveDeviceRelayInfoCallback   = std::function<void(const xRelayServerInfoBase & Info)>;

public:
    bool Init(xIoContext * ICP, const xNetAddress & ServerListDownloadAddress);
    void Clean();
    void Tick(uint64_t NowMS);

    xOnNewDeviceRelayInfoCallback      OnNewDeviceRelayInfoCallback      = Ignore<const xRelayServerInfoBase &>;
    xOnNewDeviceRelayInfoErrorCallback OnNewDeviceRelayInfoErrorCallback = Ignore<const xRelayServerInfoBase &>;
    xOnRemoveDeviceRelayInfoCallback   OnRemoveDeviceRelayInfoCallback   = Ignore<const xRelayServerInfoBase &>;

private:
    bool OnRelayInfoDispatcherPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    bool OnBroadcastRelayInfo(ubyte * PayloadPtr, size_t PayloadSize);
    bool OnBroadcastRelayOffline(ubyte * PayloadPtr, size_t PayloadSize);

    void InsertOrKeepAliveDeviceRelayInfo(const xRelayServerInfoBase & Info);
    void RemoveDeviceRelayInfo(xRelayServerInfo * RSI);

private:
    xTicker                                  Ticker;
    xClientWrapper                           RelayInfoDispatcherClient;
    xRelayInfoDispatcherServerInfoDownloader DispatcherServerInfoDownloader;

    std::map<uint64_t, uint64_t>      DeviceRelayServerIdLocalMap;
    xRelayInfoTimeoutList             RelayInfoTimeoutList;
    xIndexedStorage<xRelayServerInfo> DeviceRelayInfoPool;
};