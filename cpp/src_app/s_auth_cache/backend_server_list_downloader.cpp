#include "./backend_server_list_downloader.hpp"

void xAC_BackendServerListDownloader::OnTick(uint64_t NowMS) {
    if (NowMS - BackendServerUpdateTimestampMS >= UPDATE_SERVER_LIST_TIMEOUT_MS) {
        BackendServerUpdateTimestampMS = NowMS;
        PostUpdateServerListRequest();
    }
}

void xAC_BackendServerListDownloader::PostUpdateServerListRequest() {
    if (!IsOpen()) {
        return;
    }
    auto R    = xPP_DownloadBackendServerList();
    R.Version = BackendServerListVersion;
    PostMessage(Cmd_DownloadBackendServerList, 0, R);
}

void xAC_BackendServerListDownloader::OnServerConnected() {
    PostUpdateServerListRequest();
}

bool xAC_BackendServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DownloadBackendServerListResp:
            return OnDownloadBackendServerListResp(PayloadPtr, PayloadSize);
        default:
            Logger->E("Invalid command");
            return false;
    }
    return true;
}

void xAC_BackendServerListDownloader::OnServerClose() {
    Reset(BackendServerListVersion);
}

bool xAC_BackendServerListDownloader::OnDownloadBackendServerListResp(ubyte * PayloadPtr, size_t PayloadSize) {
    Logger->I("OnDownloadBackendServerListResp");
    auto R = xPP_DownloadBackendServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("Invalid protocol");
        return false;
    }
    if (R.Version == BackendServerListVersion) {
        return true;
    }

    auto OldList             = std::move(BackendServerList);
    BackendServerList        = std::move(R.ServerAddressList);
    BackendServerListVersion = R.Version;

    if (!UpdateCallback) {
        return true;
    }

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

    UpdateCallback(BackendServerListVersion, BackendServerList, Add, Rem);
    return true;
    //
}