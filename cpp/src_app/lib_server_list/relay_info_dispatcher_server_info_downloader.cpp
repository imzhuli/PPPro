#include "./relay_info_dispatcher_server_info_downloader.hpp"

#include "./device_state_relay_server_list_downloader.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xRelayInfoDispatcherServerInfoDownloader::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    auto TimeoutMS = NowMS - LastUpdateTimestampMS;
    if (EnableQuickUpdate && TimeoutMS < QuickUpdateTimeoutMS) {
        return;
    }
    if (TimeoutMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostDownloadRelayInfoDispatcherServerInfoRequest();
        return;
    }
}

void xRelayInfoDispatcherServerInfoDownloader::OnServerConnected() {
    Reset(QuickUpdateCount);
    Reset(EnableQuickUpdate, true);
    PostDownloadRelayInfoDispatcherServerInfoRequest();
}

void xRelayInfoDispatcherServerInfoDownloader::OnServerClose() {
    Reset(ServerInfo);
    Reset(QuickUpdateCount);
    Reset(EnableQuickUpdate, true);
}

bool xRelayInfoDispatcherServerInfoDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DownloadRelayInfoDispatcherServerResp:
            return OnDownloadRelayInfoDispatcherServerInfo(RequestId, PayloadPtr, PayloadSize);
        default:
            return false;
    }
    return true;
}

bool xRelayInfoDispatcherServerInfoDownloader::OnDownloadRelayInfoDispatcherServerInfo(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadRelayInfoDispatcherServerResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (R.ServerInfo == ServerInfo) {
        return true;
    }
    if (R.ServerInfo.ServerId) {
        EnableQuickUpdate = false;
    }

    ServerInfo = R.ServerInfo;
    OnUpdateServerInfoCallback(ServerInfo);
    return true;
}

void xRelayInfoDispatcherServerInfoDownloader::PostDownloadRelayInfoDispatcherServerInfoRequest() {
    auto R = xPP_DownloadRelayInfoDispatcherServer();
    PostMessage(Cmd_DownloadRelayInfoDispatcherServer, 0, R);
    if (EnableQuickUpdate) {
        if (QuickUpdateCount < 15) {
            ++QuickUpdateCount;
        } else {
            EnableQuickUpdate = false;
        }
    }
    LastUpdateTimestampMS = Ticker();
}
