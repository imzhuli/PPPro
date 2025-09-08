#pragma once
#include "../lib_backend_connection/backend_connection_pool.hpp"
#include "../lib_utils/all.hpp"
#include "../lib_utils/cache_manager.hpp"

#include <pp_common/_.hpp>

struct xAC_AuthService final {

    bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
    void Tick(uint64_t NowMS);
    void Clean();

    bool OnClientPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void PostResposne(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, const xClientAuthResult * CacheNode);

    void OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
    void OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

    void UpdateBackendAuthInfo(const std::string & AppKey, const std::string & AppSecret);
    void UpdateBackendServerList(const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed);  // server list must be sorted and unique

    //
    xTcpService            TcpService;
    xBackendConnectionPool BackendPool;
    xCacheManager          CacheManager;

    std::string AppKey;
    std::string AppSecret;

    uint64_t LastOutputLocalAuditTimestampMS = 0;
    size_t   NewCachedResultCount            = 0;
    size_t   DeleteCachedResultCount         = 0;
};
