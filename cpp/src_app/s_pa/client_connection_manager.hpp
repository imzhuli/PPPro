#pragma once
#include "./client_base.hpp"

#include <pp_common/_.hpp>

static constexpr const uint64_t MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS = 3'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS = 90'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_ID_COUNT        = 100'000;

class xPA_ClientConnection;

class xPA_ClientConnectionManager
    : public xTcpServer::iListener
    , public xTcpConnection::iListener {
public:
    void   Tick(uint64_t NowMS);
    void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
    void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override {}
    void   OnFlush(xTcpConnection * TcpConnectionPtr) override {}
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override { return DataSize; }
};

extern xPA_ClientConnection * GetClientConnection(uint64_t ClientConnectionId);
extern void                   KeepAliveClientConnection(xPA_ClientConnection * ClientConnection);
extern void                   CloseClientConnection(xPA_ClientConnection * ClientConnection);
extern void                   RemoveIdleClientConnections();
extern void                   DeferKillClientConnection(xPA_ClientConnection * ClientConnection);
extern void                   ClientManagerTick(uint64_t /* ignored */);
