#pragma once
#include "./client_base.hpp"

#include <pp_common/base.hpp>

static constexpr const uint64_t MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS = 3'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS = 90'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_ID_COUNT        = 100'000;

class xPA_ClientConnectionManager
    : public xTcpServer::iListener
    , public xTcpConnection::iListener {
public:
    bool Init(xIoContext * ICP);
    void Clean();
    void Tick(uint64_t NowMS);

    auto GetConnectionById(uint64_t ConnectionId) -> xPA_ClientConnection *;
    void KillConnection(xPA_ClientConnection * CP);

    void OnAuthResult(uint64_t RequestContextId, const xClientAuthResult & AuthResult);

protected:
    void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
    void   OnConnected(xTcpConnection * TcpConnectionPtr) override {}
    void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

private:
    void KeepAlive(xPA_ClientConnection * CP);

    void DoCreateConnection(xSocket && Native);
    void DoDeleteConnection(xPA_ClientConnection * CP);

    xIoContext *                            ICP = nullptr;
    xTicker                                 Ticker;
    xIndexedStorage<xPA_ClientConnection *> ConnectionIdPool;

    xPA_ClientConnectionIdleList AuthConnectionList;
    xPA_ClientConnectionIdleList IdleConnectionList;
    xPA_ClientConnectionKillList KillConnectionList;
};
