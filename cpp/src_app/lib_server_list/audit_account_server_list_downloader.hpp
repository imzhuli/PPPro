#pragma once
#include <pp_common/base.hpp>

class xAuditAccountServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(AuditAccountServerListVersionTimestampMS); }

    using xUpdateAuditAccountServerListCallback = std::function<void(const std::vector<xServerInfo> &)>;
    void SetUpdateAuditAccountServerListCallback(const xUpdateAuditAccountServerListCallback & CB) { UpdateAuditAccountServerListCallback = CB; }

private:
    bool OnAuditAccountServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostAuditAccountServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint64_t                              AuditAccountServerListVersionTimestampMS = 0;
    std::vector<xServerInfo>              AuditAccountSortedServerInfoList;
    xUpdateAuditAccountServerListCallback UpdateAuditAccountServerListCallback;
};