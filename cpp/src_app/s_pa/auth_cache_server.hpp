#pragma once
#include "../lib_utils/all.hpp"

// class xPA_AuthCacheLocalServer {
// public:
//     bool Init(xIoContext * ICP);
//     void Clean();
//     void Tick(uint64_t NowMS);

//     void UpdateServerList(const std::vector<xServerInfo> & ServerList);
//     void PostAuthRequest(uint64_t RequestContextId, const std::string & AuthKey);

//     using xOnAuthCacheResultCallback = std::function<void(uint64_t RequestContextId, const xClientAuthResult & AuthResult)>;
//     void SetOnAuthCacheResultCallback(auto && CB) { this->OnAuthCacheResultCallback = std::forward<decltype(CB)>(CB); }

// private:
//     bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

// private:
//     xClientPoolWrapper ClientHashPool;
//     //
//     std::function<void(uint64_t RequestContextId, const xClientAuthResult & AuthResult)> OnAuthCacheResultCallback = IgnoreAuthCacheResult;
//     //
//     static void IgnoreAuthCacheResult(uint64_t RequestContextId, const xClientAuthResult & AuthResult) {}
// };
