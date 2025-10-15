#include "./audit_target_server_list_downloader.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xAuditTargetServerListDownloader::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (NowMS - LastUpdateTimestampMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostAuditTargetServerListRequest();
        LastUpdateTimestampMS = Ticker();
        return;
    }
}

void xAuditTargetServerListDownloader::OnServerConnected() {
    PostAuditTargetServerListRequest();
    LastUpdateTimestampMS = Ticker();
}

bool xAuditTargetServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    switch (CommandId) {
        case Cmd_DownloadAuditTargetServerListResp:
            return OnAuditTargetServerList(RequestId, PayloadPtr, PayloadSize);
        default:
            return false;
    }

    return true;
}

bool xAuditTargetServerListDownloader::OnAuditTargetServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    auto R = xPP_DownloadAuditTargetServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (AuditTargetServerListVersion == R.Version) {
        return true;
    }

    AuditTargetSortedServerInfoList = std::move(R.ServerInfoList);
    std::sort(AuditTargetSortedServerInfoList.begin(), AuditTargetSortedServerInfoList.end(), [](auto & lhs, auto & rhs) { return lhs.ServerId < rhs.ServerId; });
    AuditTargetServerListVersion = R.Version;

    OnUpdateAuditTargetServerListCallback(AuditTargetServerListVersion, AuditTargetSortedServerInfoList);
    return true;
}

void xAuditTargetServerListDownloader::PostAuditTargetServerListRequest() {
    auto R    = xPP_DownloadAuditTargetServerList();
    R.Version = AuditTargetServerListVersion;
    PostMessage(Cmd_DownloadAuditTargetServerList, 0, R);
}
