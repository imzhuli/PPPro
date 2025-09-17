#include "./client_connection_manager.hpp"

#include "../lib_utils/service_common.hpp"
#include "_global.hpp"

struct xPA_ClientConnectionKillNode : xListNode {};
static xList<xPA_ClientConnectionKillNode> ClientConnectionKillList;
struct xPA_ClientConnectionIdleNode : xListNode {
    uint64_t LastActiveTimestampMS = 0;
};
static xList<xPA_ClientConnectionIdleNode> ClientConnectionIdleList;
static xList<xPA_ClientConnectionIdleNode> ClientConnectionClosingList;

class xPA_ClientConnection
    : public xel::xTcpConnection
    , public xPA_ClientConnectionKillNode
    , public xPA_ClientConnectionIdleNode {
public:
    xIndexId Id;
    bool     ClosingMark = false;
    bool     DeleteMark  = false;
};

static auto ClientConnectionIdPool      = xel::xIndexedStorage<xPA_ClientConnection *>();
static auto ClientConnectionIdPoolGuard = xel::xScopeGuard([] { RuntimeAssert(ClientConnectionIdPool.Init(20'0000)); }, [] { ClientConnectionIdPool.Clean(); });

void xPA_ClientConnectionManager::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
    auto Id = ClientConnectionIdPool.Acquire();
    if (!Id) {
        XelCloseSocket(std::move(NativeHandle));
        return;
    }
    auto P = new (std::nothrow) xPA_ClientConnection;
    if (!P) {
        ClientConnectionIdPool.Release(Id);
        return;
    }
    if (!P->Init(TcpServerPtr->GetIoContextPtr(), std::move(NativeHandle), this)) {
        ClientConnectionIdPool.Release(Id);
        delete P;
        return;
    }
    ClientConnectionIdPool[Id] = P;
    P->Id                      = Id;
    KeepAliveClientConnection(P);
}

static void DestroyClientConnection(xPA_ClientConnection * ClientConnection) {
    ClientConnectionIdPool.Release(ClientConnection->Id);
    ClientConnection->Clean();
    delete ClientConnection;
}

void ClientManagerTick(uint64_t) {
    RemoveIdleClientConnections();
    while (auto P = (xPA_ClientConnection *)ClientConnectionKillList.PopHead()) {
        DestroyClientConnection(P);
    }
}

xPA_ClientConnection * GetClientConnection(uint64_t ClientConnectionId) {
    auto PP = ClientConnectionIdPool.CheckAndGet(ClientConnectionId);
    if (!PP) {
        return nullptr;
    }
    return *PP;
}

void KeepAliveClientConnection(xPA_ClientConnection * ClientConnection) {
    if (ClientConnection->DeleteMark || ClientConnection->ClosingMark) {
        return;
    }
    ClientConnection->LastActiveTimestampMS = ServiceTicker();
    ClientConnectionIdleList.GrabTail(*ClientConnection);
}

void CloseClientConnection(xPA_ClientConnection * ClientConnection) {
    if (ClientConnection->DeleteMark) {
        return;
    }
    if (!ClientConnection->HasPendingWrites()) {
        DeferKillClientConnection(ClientConnection);
        return;
    }
    ClientConnection->ClosingMark           = true;
    ClientConnection->LastActiveTimestampMS = ServiceTicker();
    ClientConnectionClosingList.GrabTail(*ClientConnection);
}

void RemoveIdleClientConnections() {
    auto KillTimepoint = ServiceTicker() - MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS;
    auto IdlePred      = [KillTimepoint](const xPA_ClientConnectionIdleNode & C) { return C.LastActiveTimestampMS <= KillTimepoint; };
    while (auto P = (xPA_ClientConnection *)ClientConnectionIdleList.PopHead(IdlePred)) {
        DestroyClientConnection(P);
    }
}

void DeferKillClientConnection(xPA_ClientConnection * ClientConnection) {
    ClientConnection->DeleteMark = true;
    ClientConnectionKillList.GrabTail(*ClientConnection);
}
