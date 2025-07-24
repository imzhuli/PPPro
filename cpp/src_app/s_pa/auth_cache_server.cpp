#include "./auth_cache_server.hpp"

#include "../lib_server_util/all.hpp"

void xPA_AuthCacheServerListDownloader::OnTick(uint64_t NowMS) {
    if (NowMS - LastUpdateTimestampMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostDownloadRequest();
        LastUpdateTimestampMS = NowMS;
        return;
    }
}

bool xPA_AuthCacheServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_DownloadAuthCacheServerListResp) {
        return false;
    }

    auto R = xPP_DownloadAuthCacheServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (ServerListVersion == R.Version) {
        return true;
    }
    SortedServerInfoList = std::move(R.ServerInfoList);
    std::sort(SortedServerInfoList.begin(), SortedServerInfoList.end(), [](xServerInfo & lhs, xServerInfo & rhs) { return lhs.ServerId < rhs.ServerId; });
    ServerListVersion = R.Version;

    if (UpdateServerListCallback) {
        UpdateServerListCallback(SortedServerInfoList);
    }
    return true;
}

void xPA_AuthCacheServerListDownloader::PostDownloadRequest() {
    auto R    = xPP_DownloadAuthCacheServerList();
    R.Version = ServerListVersion;
    PostMessage(Cmd_DownloadAuthCacheServerList, 0, R);
}

/////////////////////////////////

bool xPA_AuthCacheLocalServer::Init(xIoContext * ICP) {
    assert(ICP);
    if (!ClientHashPool.Init(ICP)) {
        return false;
    }
    ClientHashPool.SetOnPacketCallback([this](xMessagePoster * Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) -> bool {
        return OnServerPacket(CommandId, RequestId, PayloadPtr, PayloadSize);
    });

    return true;
}

void xPA_AuthCacheLocalServer::Clean() {
    ClientHashPool.Clean();
}

void xPA_AuthCacheLocalServer::Tick(uint64_t NowMS) {
    ClientHashPool.Tick(NowMS);
}

void xPA_AuthCacheLocalServer::UpdateServerList(const std::vector<xServerInfo> & ServerList) {
    auto AddressList = std::vector<xNetAddress>();
    for (auto S : ServerList) {
        AddressList.push_back(S.Address);
    }
    ClientHashPool.UpdateServerList(AddressList);
}

void xPA_AuthCacheLocalServer::PostAuthRequest(uint64_t RequestContextId, const std::string & AuthKey) {
    auto Hash    = HashString(AuthKey);
    auto Req     = xPP_QueryAuthCache();
    Req.UserPass = AuthKey;

    ClientHashPool.PostMessage(Hash, Cmd_AuthService_QueryAuthCache, RequestContextId, Req);
}

bool xPA_AuthCacheLocalServer::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    DEBUG_LOG("CommondId=%" PRIx32 ", RequestId:%" PRIx64 " Data=\n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
    if (CommandId != Cmd_AuthService_QueryAuthCacheResp) {
        Logger->E("Invalid server response command");
        return false;
    }
    auto Resp = xPP_QueryAuthCacheResp();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("Invalid server response protocol");
        return true;
    }
    DoCallback(RequestId, Resp.Result);
    return true;
}

void xPA_AuthCacheLocalServer::DoCallback(uint64_t RequestContextId, const xClientAuthResult & AuthResult) {
    if (Callback) {
        Callback(RequestContextId, AuthResult);
    }
}
