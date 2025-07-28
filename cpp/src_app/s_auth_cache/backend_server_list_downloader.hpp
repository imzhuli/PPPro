#pragma once
#include "../lib_server_util/all.hpp"

class xAC_BackendServerListDownloader : xClient {
public:
    static constexpr const uint64_t UPDATE_SERVER_LIST_TIMEOUT_MS = 3 * 60'000;

    using xClient::Init;
    void Clean() {
        Reset(BackendServerList);
        Reset(BackendServerListVersionTimestampMS);
        xClient::Clean();
    }
    using xClient::Tick;

    using xUpdateCallback =
        std::function<void(uint32_t Version, const std::vector<xNetAddress> & FullList, const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed)>;
    void SetUpdateCallback(xUpdateCallback Callback) { this->UpdateCallback = Callback; }

protected:
    void OnTick(uint64_t NowMS) override;
    void OnServerConnected() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override;

    void PostUpdateServerListRequest();
    bool OnDownloadBackendServerListResp(ubyte * PayloadPtr, size_t PayloadSize);

private:
    uint64_t                 BackendServerListVersionTimestampMS = 0;
    std::vector<xNetAddress> BackendServerList;
    xUpdateCallback          UpdateCallback;
};
