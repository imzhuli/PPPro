#pragma once
#include <pp_common/base.hpp>

class xAuthCacheServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(AuthCacheServerListVersionTimestampMS); }

    using xUpdateAuthCacheServerListCallback = std::function<void(const std::vector<xServerInfo> &)>;
    void SetUpdateAuthCacheServerListCallback(const xUpdateAuthCacheServerListCallback & CB) { UpdateAuthCacheServerListCallback = CB; }

private:
    bool OnAuthCacheServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostAuthCacheServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint64_t                           AuthCacheServerListVersionTimestampMS = 0;
    std::vector<xServerInfo>           AuthCacheSortedServerInfoList;
    xUpdateAuthCacheServerListCallback UpdateAuthCacheServerListCallback;
};