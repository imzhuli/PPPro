#pragma once
#include <pp_common/_.hpp>

class xRelayInfoDispatcherServerInfoDownloader final : public xClient {

public:
    static constexpr const uint64_t QuickUpdateTimeoutMS = 5'000;
    static constexpr const uint64_t UpdateTimeoutMS      = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override;

    using xOnUpdateServerInfoCallback                      = std::function<void(const xRelayInfoDispatcherServerInfo &)>;
    xOnUpdateServerInfoCallback OnUpdateServerInfoCallback = NOOP<const xRelayInfoDispatcherServerInfo &>;

private:
    bool OnDownloadRelayInfoDispatcherServerInfo(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostDownloadRelayInfoDispatcherServerInfoRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;
    uint64_t QuickUpdateCount      = 0;
    bool     EnableQuickUpdate     = true;

    xRelayInfoDispatcherServerInfo ServerInfo = {};
};
