#pragma once
#include <map>
#include <pp_common/_.hpp>
#include <server_arch/client_pool.hpp>

struct xBackendServerInfo {
    xNetAddress Address;
    uint64_t    ConnectionId;
};
inline std::strong_ordering operator<=>(const xBackendServerInfo & lhs, const xBackendServerInfo & rhs) {
    return lhs.Address <=> rhs.Address;
}
inline bool operator==(const xBackendServerInfo & lhs, const xBackendServerInfo & rhs) {
    return lhs.Address == rhs.Address;
}

class xBackendConnectionPool : protected xClientPool {

public:
    bool Init(xIoContext * ICP, size_t MaxConnectionCount);
    void Clean();
    using xClientPool::PostMessage;
    using xClientPool::Tick;

    std::string GetLocalAudit();
    void        ResetLocalAudit();

    using xOnBackendPacketCallback                   = std::function<void(xPacketCommandId, xPacketRequestId, ubyte *, size_t)>;
    xOnBackendPacketCallback OnBackendPacketCallback = Noop<>;

public:
    uint64_t AddServer(const xNetAddress & Address, const std::string & AppKey, const std::string & AppSecret);
    void     RemoveServer(const xNetAddress & Address);

private:
    void OnServerConnected(xClientConnection & CC) override;
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerClose(xClientConnection & CC) override;
    bool OnCmdBackendChallengeResp(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

    //
    struct xBackendConnectionContext {
        std::string AppKey;
        std::string AppSecret;
        bool        IsChallengeReady = false;
    };
    std::vector<xBackendConnectionContext> ContextList;
    std::vector<xBackendServerInfo>        SortedServerList;

private:  // audit
    size_t TotalAddedServer           = 0;
    size_t TotalRemovedServer         = 0;
    size_t TotalAddingServerConflict  = 0;
    size_t TotalRemovingServerFailure = 0;
};
