#pragma once
#include <pp_common/_.hpp>

class xAuditAccountServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(AuditAccountServerListVersion); }

    using xUpdateAuditAccountServerListCallback = std::function<void(uint32_t Version, const std::vector<xServerInfo> & ServerList)>;
    void SetOnUpdateAuditAccountServerListCallback(const xUpdateAuditAccountServerListCallback & CB) { UpdateAuditAccountServerListCallback = CB; }

private:
    bool OnAuditAccountServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostAuditAccountServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint32_t                              AuditAccountServerListVersion = 0;
    std::vector<xServerInfo>              AuditAccountSortedServerInfoList;
    xUpdateAuditAccountServerListCallback UpdateAuditAccountServerListCallback;
};