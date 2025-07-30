#pragma once
#include "../lib_server_util/all.hpp"

#include <pp_common/_.hpp>
#include <typeinfo>
using std::type_info;

class xPA_ClientConnection;
class xPA_ClientConnectionManager;
class xPA_ClientConnection;
extern xPA_ClientConnection * UpCast(xTcpConnection * CP);

struct xPA_ClientStateHandler {
    virtual void   OnEntry(xPA_ClientConnection * Client) { /* DEBUG_LOG("StateEntry: %s", typeid(*this).name()); */ }
    virtual size_t OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) { return DataSize; }
    virtual void   OnAuthResult(xPA_ClientConnection * Client, const xClientAuthResult & Result) { DEBUG_LOG(""); };
    virtual void   OnExit(xPA_ClientConnection * Client) { /* DEBUG_LOG("StateExit: %s", typeid(*this).name()); */ }
};
struct xPA_ClientStateDataBase : xVBase {};
struct xPA_ClientStateContext final {
    xPA_ClientStateHandler *                 CurrentHandler = nullptr;
    xPA_ClientStateHandler *                 NextHandler    = nullptr;
    std::unique_ptr<xPA_ClientStateDataBase> StateData      = {};
};
extern void ScheduleClientStateChange(xPA_ClientConnection * Client, xPA_ClientStateHandler * NextStateHandler);
extern void InitClientState(xPA_ClientConnection * Client);
extern bool UpdateClientState(xPA_ClientConnection * Client);
extern void FinalClientState(xPA_ClientConnection * Client);

extern xPA_ClientStateHandler * TestProxyTypetState;
extern xPA_ClientStateHandler * CloseClientStateHandler;
extern xPA_ClientStateHandler * S5ChallengeStateHandler;
extern xPA_ClientStateHandler * S5WaitForAuthInfoStateHandler;
extern xPA_ClientStateHandler * S5WaitForAuthResultStateHandler;
extern xPA_ClientStateHandler * HttpChallengeStateHandler;

struct xPA_ClientConnectionIdleNode : xListNode {
    uint64_t LastActiveTimestampMS = 0;
};
using xPA_ClientConnectionIdleList = xList<xPA_ClientConnectionIdleNode>;

struct xPA_ClientConnectionKillNode : xListNode {};
using xPA_ClientConnectionKillList = xList<xPA_ClientConnectionKillNode>;

struct xPA_SharedClientStateContext : xVBase {};

struct xPA_ClientConnectionAudit {
    uint64_t AuditId;
    uint64_t StartupTimestamp;
    size_t   UploadSize;
    size_t   DownloadSize;
};

class xPA_ClientConnection final
    : private xTcpConnection
    , public xPA_ClientConnectionIdleNode
    , public xPA_ClientConnectionKillNode {
public:
    friend xPA_ClientConnection * UpCast(xTcpConnection * CP);

    using xTcpConnection::Clean;
    using xTcpConnection::Init;

    using xTcpConnection::GetRemoteAddress;
    using xTcpConnection::IsOpen;

    xPA_ClientConnectionManager * Owner        = nullptr;
    xIndexId                      ConnectionId = {};
    xPA_ClientStateContext        StateContext;
    xPA_ClientConnectionAudit     Audit = {};

    void PostData(const void * DataPtr, size_t DataSize) {
        Audit.DownloadSize += DataSize;
        xTcpConnection::PostData(DataPtr, DataSize);
    }
};
inline auto GetClientStateHandler(xPA_ClientConnection * CP) {
    return CP->StateContext.CurrentHandler;
}

inline auto UpCast(xPA_ClientConnectionIdleNode * CP) {
    return static_cast<xPA_ClientConnection *>(CP);
}

inline auto UpCast(xPA_ClientConnectionKillNode * CP) {
    return static_cast<xPA_ClientConnection *>(CP);
}

struct xPA_ClientStateHandler_TestProxyType : xPA_ClientStateHandler {
    size_t OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) override;
};
struct xPA_ClientStateHandler_CloseStateHandler : xPA_ClientStateHandler {
    void OnEntry(xPA_ClientConnection * Client) override;
};
struct xPA_ClientStateHandler_S5_Challenge : xPA_ClientStateHandler {
    size_t OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) override;
};
struct xPA_ClientStateHandler_S5_WaitForAuthInfo : xPA_ClientStateHandler {
    size_t OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) override;
};
struct xPA_ClientStateHandler_S5_WaitForAuthResult : xPA_ClientStateHandler {
    size_t OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) override;
};
struct xPA_ClientStateHandler_Http_Challenge : xPA_ClientStateHandler {
    size_t OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) override;
};

//////////

extern void KillConnection(xPA_ClientConnection * CP);
