#include "./auth_cache_server_list_downloader.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xAuthCacheServerListDownloader::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (NowMS - LastUpdateTimestampMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostAuthCacheServerListRequest();
        LastUpdateTimestampMS = Ticker();
        return;
    }
}

void xAuthCacheServerListDownloader::OnServerConnected() {
    PostAuthCacheServerListRequest();
    LastUpdateTimestampMS = Ticker();
}

bool xAuthCacheServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    switch (CommandId) {
        case Cmd_DownloadAuthCacheServerListResp:
            return OnAuthCacheServerList(RequestId, PayloadPtr, PayloadSize);
        default:
            return false;
    }

    return true;
}

bool xAuthCacheServerListDownloader::OnAuthCacheServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    // DEBUG_LOG("%s", HexShow(PayloadPtr, PayloadSize).c_str());

    auto R = xPP_DownloadAuthCacheServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (AuthCacheServerListVersion == R.Version) {
        return true;
    }

    AuthCacheSortedServerInfoList = std::move(R.ServerInfoList);
    std::sort(AuthCacheSortedServerInfoList.begin(), AuthCacheSortedServerInfoList.end(), [](auto & lhs, auto & rhs) { return lhs.ServerId < rhs.ServerId; });
    AuthCacheServerListVersion = R.Version;

    OnUpdateAuthCacheServerListCallback(AuthCacheServerListVersion, AuthCacheSortedServerInfoList);
    return true;
}

void xAuthCacheServerListDownloader::PostAuthCacheServerListRequest() {
    auto R    = xPP_DownloadAuthCacheServerList();
    R.Version = AuthCacheServerListVersion;
    PostMessage(Cmd_DownloadAuthCacheServerList, 0, R);
}
