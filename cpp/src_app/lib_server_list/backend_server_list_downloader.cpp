#include "./backend_server_list_downloader.hpp"

void xBackendServerListDownloader::OnTick(uint64_t NowMS) {
    if (NowMS - LastUpdateTimestampMS >= UPDATE_SERVER_LIST_TIMEOUT_MS) {
        LastUpdateTimestampMS = NowMS;
        PostUpdateServerListRequest();
    }
}

void xBackendServerListDownloader::PostUpdateServerListRequest() {
    if (!IsOpen()) {
        return;
    }
    auto R    = xPP_DownloadBackendServerList();
    R.Version = BackendServerListVersion;
    PostMessage(Cmd_DownloadBackendServerList, 0, R);
}

void xBackendServerListDownloader::OnServerConnected() {
    PostUpdateServerListRequest();
}

bool xBackendServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DownloadBackendServerListResp:
            return OnDownloadBackendServerListResp(PayloadPtr, PayloadSize);
        default:
            return false;
    }
    return true;
}

void xBackendServerListDownloader::OnServerClose() {
    Reset(BackendServerListVersion);
}

bool xBackendServerListDownloader::OnDownloadBackendServerListResp(ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadBackendServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (R.Version == BackendServerListVersion) {
        return true;
    }

    auto OldList             = std::move(BackendServerList);
    BackendServerList        = std::move(R.ServerAddressList);
    BackendServerListVersion = R.Version;

    auto Add = std::vector<xNetAddress>();
    auto Rem = std::vector<xNetAddress>();

    size_t IOld = 0;
    size_t INew = 0;
    while (IOld < OldList.size() && INew < BackendServerList.size()) {
        auto & O = OldList[IOld];
        auto & N = BackendServerList[INew];
        if (O < N) {
            Rem.push_back(O);
            ++IOld;
            continue;
        }
        if (N < O) {
            Add.push_back(N);
            ++INew;
            continue;
        }
        ++IOld;
        ++INew;
    }
    for (; INew < BackendServerList.size(); ++INew) {
        Add.push_back(BackendServerList[INew]);
    }
    for (; IOld < OldList.size(); ++IOld) {
        Rem.push_back(OldList[IOld]);
    }

    OnUpdateCallback(BackendServerListVersion, BackendServerList, Add, Rem);
    return true;
    //
}