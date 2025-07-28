#include "./audit_account_server_list_downloader.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xAuditAccountServerListDownloader::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (NowMS - LastUpdateTimestampMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostAuditAccountServerListRequest();
        LastUpdateTimestampMS = Ticker();
        return;
    }
}

void xAuditAccountServerListDownloader::OnServerConnected() {
    PostAuditAccountServerListRequest();
    LastUpdateTimestampMS = Ticker();
}

bool xAuditAccountServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    switch (CommandId) {
        case Cmd_DownloadAuditAccountServerListResp:
            return OnAuditAccountServerList(RequestId, PayloadPtr, PayloadSize);
        default:
            return false;
    }

    return true;
}

bool xAuditAccountServerListDownloader::OnAuditAccountServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    // DEBUG_LOG("%s", HexShow(PayloadPtr, PayloadSize).c_str());

    auto R = xPP_DownloadAuditAccountServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (AuditAccountServerListVersion == R.Version) {
        return true;
    }

    AuditAccountSortedServerInfoList = std::move(R.ServerInfoList);
    std::sort(AuditAccountSortedServerInfoList.begin(), AuditAccountSortedServerInfoList.end(), [](auto & lhs, auto & rhs) { return lhs.ServerId < rhs.ServerId; });
    AuditAccountServerListVersion = R.Version;

    if (UpdateAuditAccountServerListCallback) {
        UpdateAuditAccountServerListCallback(AuditAccountServerListVersion, AuditAccountSortedServerInfoList);
    }
    return true;
}

void xAuditAccountServerListDownloader::PostAuditAccountServerListRequest() {
    auto R    = xPP_DownloadAuditAccountServerList();
    R.Version = AuditAccountServerListVersion;
    PostMessage(Cmd_DownloadAuditAccountServerList, 0, R);
}
