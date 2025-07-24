#pragma once
#include "../lib_server_util/all.hpp"

class xPA_AuthCacheServerListDownloader : public xClient {
public:
    static constexpr const uint64_t UpdateTimeoutMS = 5 * 60'000;

    void OnTick(uint64_t NowMS) override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose() override { Reset(ServerListVersion); }

    void SetUpdateServerListCallback(auto && CB) { this->UpdateServerListCallback = std::forward<decltype(CB)>(CB); }

private:
    void PostDownloadRequest();

private:
    uint64_t                 LastUpdateTimestampMS = 0;
    uint32_t                 ServerListVersion     = 0;
    std::vector<xServerInfo> SortedServerInfoList;

    std::function<void(const std::vector<xServerInfo> &)> UpdateServerListCallback;
};

class xPA_AuthCacheLocalServer {
public:
    bool Init(xIoContext * ICP);
    void Clean();
    void Tick(uint64_t NowMS);

    void UpdateServerList(const std::vector<xServerInfo> & ServerList);
    void SetCallback(auto && CB) { this->Callback = std::forward<decltype(CB)>(CB); }
    void PostAuthRequest(uint64_t RequestContextId, const std::string & AuthKey);

protected:
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void DoCallback(uint64_t RequestContextId, const xClientAuthResult & AuthResult);

private:
    xClientPoolWrapper ClientHashPool;

    std::function<void(uint64_t RequestContextId, const xClientAuthResult & AuthResult)> Callback;
};
