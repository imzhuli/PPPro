#pragma once
#include <pp_common/base.hpp>

class xDeviceSelectorDispatcherServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(DeviceSelectorDispatcherServerListVersionTimestampMS); }

    using xUpdateDeviceSelectorDispatcherServerListCallback = std::function<void(const std::vector<xDeviceSelectorDispatcherInfo> &)>;
    void SetUpdateDeviceSelectorDispatcherServerListCallback(const xUpdateDeviceSelectorDispatcherServerListCallback & CB) {
        UpdateDeviceSelectorDispatcherServerListCallback = CB;
    }

private:
    bool OnDeviceSelectorDispatcherServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostDeviceSelectorDispatcherServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint64_t                                          DeviceSelectorDispatcherServerListVersionTimestampMS = 0;
    std::vector<xDeviceSelectorDispatcherInfo>        DeviceSelectorDispatcherSortedServerInfoList;
    xUpdateDeviceSelectorDispatcherServerListCallback UpdateDeviceSelectorDispatcherServerListCallback;
};