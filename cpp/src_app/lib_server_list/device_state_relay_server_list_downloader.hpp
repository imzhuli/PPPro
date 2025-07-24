#pragma once
#include <pp_common/base.hpp>

class xDeviceStateRelayServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(DeviceStateRelayServerListVersion); }

    using xUpdateDeviceStateRelayServerListCallback = std::function<void(const std::vector<xDeviceStateRelayServerInfo> &)>;
    void SetUpdateDeviceStateRelayServerListCallback(const xUpdateDeviceStateRelayServerListCallback & CB) { this->UpdateDeviceStateRelayServerListCallback = CB; }

private:
    bool OnDeviceStateRelayServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostDownloadDeviceStateRelayServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint32_t                                  DeviceStateRelayServerListVersion = 0;
    std::vector<xDeviceStateRelayServerInfo>  DeviceStateRelaySortedServerInfoList;
    xUpdateDeviceStateRelayServerListCallback UpdateDeviceStateRelayServerListCallback;
};
