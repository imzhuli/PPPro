#pragma once
#include "../lib_server_util/all.hpp"

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
