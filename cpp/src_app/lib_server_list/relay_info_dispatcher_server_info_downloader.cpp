#include "./relay_info_dispatcher_server_info_downloader.hpp"

#include "./device_state_relay_server_list_downloader.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xRelayInfoDispatcherServerInfoDownloader::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (NowMS - LastUpdateTimestampMS < QuickUpdateTimeoutMS) {
        return;
    }
    if (ServerInfo.ServerId && NowMS - LastUpdateTimestampMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostDownloadRelayInfoDispatcherServerInfoRequest();
        LastUpdateTimestampMS = Ticker();
        return;
    }
}

void xRelayInfoDispatcherServerInfoDownloader::OnServerConnected() {
    PostDownloadRelayInfoDispatcherServerInfoRequest();
    LastUpdateTimestampMS = Ticker();
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

    // DEBUG_LOG("%s", HexShow(PayloadPtr, PayloadSize).c_str());

    auto R = xPP_DownloadRelayInfoDispatcherServerResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (R.ServerInfo == ServerInfo) {
        return true;
    }

    ServerInfo = R.ServerInfo;
    if (UpdateRelayInfoDispatcherServerInfoCallback) {
        UpdateRelayInfoDispatcherServerInfoCallback(ServerInfo);
    }
    return true;
}

void xRelayInfoDispatcherServerInfoDownloader::PostDownloadRelayInfoDispatcherServerInfoRequest() {
    auto R = xPP_DownloadRelayInfoDispatcherServer();
    PostMessage(Cmd_DownloadRelayInfoDispatcherServer, 0, R);
}
