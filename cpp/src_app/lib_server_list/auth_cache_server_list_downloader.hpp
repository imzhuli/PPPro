#pragma once
#include <pp_common/_.hpp>

class xAuthCacheServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(AuthCacheServerListVersion); }

    using xUpdateAuthCacheServerListCallback = std::function<void(uint32_t Version, const std::vector<xServerInfo> & ServerList)>;
    void SetOnUpdateAuthCacheServerListCallback(const xUpdateAuthCacheServerListCallback & CB) { UpdateAuthCacheServerListCallback = CB; }

private:
    bool OnAuthCacheServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostAuthCacheServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint32_t                           AuthCacheServerListVersion = 0;
    std::vector<xServerInfo>           AuthCacheSortedServerInfoList;
    xUpdateAuthCacheServerListCallback UpdateAuthCacheServerListCallback = IgnoreUpdateAuthCacheServerListCallback;

    static void IgnoreUpdateAuthCacheServerListCallback(uint32_t Version, const std::vector<xServerInfo> & ServerList) {};
};