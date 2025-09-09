#pragma once
#include <pp_common/_.hpp>

class xDeviceStateRelayServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    using xOnUpdateDeviceStateRelayServerListCallback = std::function<void(uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & ServerList)>;

    xOnUpdateDeviceStateRelayServerListCallback OnUpdateDeviceStateRelayServerListCallback = Noop<>;

private:
    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(DeviceStateRelayServerListVersion); }

    bool OnDeviceStateRelayServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostDownloadDeviceStateRelayServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint32_t                                 DeviceStateRelayServerListVersion = 0;
    std::vector<xDeviceStateRelayServerInfo> DeviceStateRelaySortedServerInfoList;
};
