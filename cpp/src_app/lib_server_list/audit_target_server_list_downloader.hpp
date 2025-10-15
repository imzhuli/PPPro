#pragma once
#include <pp_common/_.hpp>

class xAuditTargetServerListDownloader : public xClient {

public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(AuditTargetServerListVersion); }

    using xOnUpdateAuditTargetServerListCallback = std::function<void(uint32_t Version, const std::vector<xServerInfo> & ServerList)>;

    xOnUpdateAuditTargetServerListCallback OnUpdateAuditTargetServerListCallback = Noop<>;

private:
    bool OnAuditTargetServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostAuditTargetServerListRequest();

private:
    xTicker  Ticker;
    uint64_t LastUpdateTimestampMS = 0;

    uint32_t                 AuditTargetServerListVersion = 0;
    std::vector<xServerInfo> AuditTargetSortedServerInfoList;
};
