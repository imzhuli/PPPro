#pragma once
#include <pp_common/_.hpp>

class xDeviceSelectorDispatcherServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(DeviceSelectorDispatcherServerListVersion); }

    using xOnUpdateDeviceSelectorDispatcherServerListCallback = std::function<void(uint32_t Version, const std::vector<xDeviceSelectorDispatcherInfo> & ServerList)>;

    xOnUpdateDeviceSelectorDispatcherServerListCallback OnUpdateDeviceSelectorDispatcherServerListCallback = Noop<>;

private:
    bool OnDeviceSelectorDispatcherServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostDeviceSelectorDispatcherServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint32_t                                   DeviceSelectorDispatcherServerListVersion    = 0;
    std::vector<xDeviceSelectorDispatcherInfo> DeviceSelectorDispatcherSortedServerInfoList = {};
};