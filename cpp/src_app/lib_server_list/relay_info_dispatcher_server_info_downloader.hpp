#pragma once
#include <pp_common/_.hpp>

class xRelayInfoDispatcherServerInfoDownloader : public xClient {

public:
    static constexpr const uint64_t QuickUpdateTimeoutMS = 30'000;
    static constexpr const uint64_t UpdateTimeoutMS      = 5 * 60'000;
    using xUpdateRelayInfoDispatcherServerInfoCallback   = std::function<void(const xRelayInfoDispatcherServerInfo &)>;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(ServerInfo); }

    void SetOnUpdateRelayInfoDispatcherServerInfoCallback(const xUpdateRelayInfoDispatcherServerInfoCallback & CB) { this->UpdateRelayInfoDispatcherServerInfoCallback = CB; }

private:
    bool OnDownloadRelayInfoDispatcherServerInfo(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostDownloadRelayInfoDispatcherServerInfoRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    xRelayInfoDispatcherServerInfo               ServerInfo = {};
    xUpdateRelayInfoDispatcherServerInfoCallback UpdateRelayInfoDispatcherServerInfoCallback;
};
