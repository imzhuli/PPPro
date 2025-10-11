#include "./client_manager.hpp"

#include "_global.hpp"

static constexpr const uint64_t MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS         = 3'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_PASSIVE_KILL_TIMEOUT_MS = 10'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS         = 90'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_ID_COUNT                = 250'000;

struct xPA_ClientConnectionListenerWrapper : xTcpConnection::iListener {
    void   OnPeerClose(xTcpConnection * CP) override { OnPAClientConnectionPeerClose(static_cast<xPA_ClientTcpConnection *>(CP)->Owner); }
    void   OnFlush(xTcpConnection * CP) override { OnPAClientConnectionFlush(static_cast<xPA_ClientTcpConnection *>(CP)->Owner); }
    size_t OnData(xTcpConnection * CP, ubyte * DataPtr, size_t DataSize) override {
        return OnPAClientConnectionData(static_cast<xPA_ClientTcpConnection *>(CP)->Owner, DataPtr, DataSize);
    }
};

struct xClientServerWrapper : xTcpServer::iListener {
    void OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
        auto C = AcceptClientConnection(std::move(NativeHandle));
        if (!C) {
            return;
        }
        OnPAClientConnectionAccepted(C);
    }
};

static xTcpServer                                   ClientTcpServer;
static xPA_ClientConnectionListenerWrapper          ClientConnectionListenerWrapper;
static xClientServerWrapper                         ClientServerWrapper;
static xel::xIndexedStorage<xPA_ClientConnection *> ClientIdManager;
static xList<xPA_ClientConnectionIdleNode>          AuthTimeoutList;
static xList<xPA_ClientConnectionIdleNode>          ClientIdleList;
static xList<xPA_ClientConnectionKillNode>          ClientPassiveKillList;
static xList<xPA_ClientConnectionKillNode>          ClientKillList;

//////////////// public functions

void InitClientManager() {
    RuntimeAssert(ClientTcpServer.Init(ServiceIoContext, ConfigTcpBindAddress, &ClientServerWrapper));
    RuntimeAssert(ClientIdManager.Init(MAX_CLIENT_CONNECTION_ID_COUNT));
}

void CleanClietManager() {
    ClientIdManager.Clean();
    ClientTcpServer.Clean();
}

void TickClientManager(uint64_t NowMS) {
    auto AuthTimeoutPred = [KillPoint = NowMS - MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS](const xPA_ClientConnectionIdleNode & N) { return N.LastActivityTimestampMS <= KillPoint; };
    while (auto P = static_cast<xPA_ClientConnection *>(AuthTimeoutList.PopHead(AuthTimeoutPred))) {
        DEBUG_LOG("AuthTimeout");
        DestroyClientConnection(P);
    }

    auto IdleTimeoutPred = [KillPoint = NowMS - MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS](const xPA_ClientConnectionIdleNode & N) { return N.LastActivityTimestampMS <= KillPoint; };
    while (auto P = static_cast<xPA_ClientConnection *>(ClientIdleList.PopHead(IdleTimeoutPred))) {
        DEBUG_LOG("IdleTimeout");
        DestroyClientConnection(P);
    }

    auto PassiveTimeoutPred = [KillPoint = NowMS - MAX_CLIENT_CONNECTION_PASSIVE_KILL_TIMEOUT_MS](const xPA_ClientConnectionKillNode & N) {
        return N.KillingScheduledTimestampMS <= KillPoint;
    };
    while (auto P = static_cast<xPA_ClientConnection *>(ClientPassiveKillList.PopHead(PassiveTimeoutPred))) {
        DEBUG_LOG("PassiveTimeout");
        DestroyClientConnection(P);
    }

    while (auto P = static_cast<xPA_ClientConnection *>(ClientKillList.PopHead())) {
        DEBUG_LOG("DirectKilled");
        DestroyClientConnection(P);
    }
}

xPA_ClientConnection * AcceptClientConnection(xSocket && NativeHandle) {
    auto C = new (std::nothrow) xPA_ClientConnection();
    if (!C) {
        XelCloseSocket(NativeHandle);
        return nullptr;
    }
    if (!C->Conn.Init(ServiceIoContext, std::move(NativeHandle), &ClientConnectionListenerWrapper)) {
        delete C;
        return nullptr;
    }
    auto Id = ClientIdManager.Acquire(C);
    if (!Id) {
        C->Conn.Clean();
        delete C;
        return nullptr;
    }
    C->ConnectionId = Id;
    DEBUG_LOG("ConnectionId: %" PRIx64 "", C->ConnectionId);
    return C;
}

void DestroyClientConnection(xPA_ClientConnection * CC) {
    DEBUG_LOG("ConnectionId: %" PRIx64 "", CC->ConnectionId);
    assert(CC == GetClientConnectionById(CC->ConnectionId));
    ClientIdManager.Release(CC->ConnectionId);
    CC->Conn.Clean();
    delete CC;
}

xPA_ClientConnection * GetClientConnectionById(uint64_t ClientConnectionId) {
    auto PP = ClientIdManager.CheckAndGet(ClientConnectionId);
    if (!PP) {
        return nullptr;
    }
    return *PP;
}

void KeepAlive(xPA_ClientConnection * CC) {
    CC->LastActivityTimestampMS = ServiceTicker();
    ClientIdleList.GrabTail(*CC);
}

void DeferKillClientConnection(xPA_ClientConnection * CC) {
    ClientKillList.GrabTail(*CC);
}

void SchedulePassiveKillClientConnection(xPA_ClientConnection * CC) {
    if (CC->State == CS_KILL_ON_FLUSH) {
        return;
    }
    CC->State = CS_KILL_ON_FLUSH;
    if (!CC->HasPendingWrites()) {
        DeferKillClientConnection(CC);
        return;
    }
    CC->KillingScheduledTimestampMS = ServiceTicker();
    ClientPassiveKillList.GrabTail(*CC);
}

void OnPAClientConnectionAccepted(xPA_ClientConnection * CC) {
    CC->LastActivityTimestampMS = ServiceTicker();
    AuthTimeoutList.AddTail(*CC);
}

void OnPAClientConnectionPeerClose(xPA_ClientConnection * CC) {
    switch (CC->State) {
        case CS_S5_READY:  // pass-through
        case CS_H_READY:   // pass-through
        case CS_C_READY:   // pass-through
            RequestRelayCloseConnection(CC->ConnectionId, CC->DeviceRelayServerRuntimeId, CC->RelaySideContextId);
            break;

        default:
            DEBUG_LOG("unhandled state on client close");
            DeferKillClientConnection(CC);
            break;
    }
}

void OnPAClientConnectionFlush(xPA_ClientConnection * CC) {
    if (CC->State == CS_KILL_ON_FLUSH) {
        DEBUG_LOG("close on flush");
        DeferKillClientConnection(CC);
    }
}

size_t OnPAClientConnectionData(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    switch (CC->State) {
        case CS_CHALLENGE:
            return OnPAC_Challenge(CC, DP, DS);
        case CS_S5_CHALLENGE:
            return OnPAC_S5_Challenge(CC, DP, DS);
        case CS_S5_WAIT_FOR_AUTH_INFO:
            return OnPAC_S5_AuthInfo(CC, DP, DS);
        case CS_S5_WAIT_FOR_AUTH_RESULT:
            return InvalidDataSize;
        case CS_S5_WAIT_FOR_TARGET_ADDRESS:
            return OnPAC_S5_TargetAddress(CC, DP, DS);
        case CS_S5_WAIT_FOR_CONECTION_ESTABLISH:
            return InvalidDataSize;
        case CS_S5_READY:
            return OnPAC_S5_PushData(CC, DP, DS);

        default:
            DEBUG_LOG("Unhandled state: %u", (unsigned)CC->State);
            return InvalidDataSize;
    }
    return DS;
}

size_t OnPAC_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    if (DP[0] == '\x05') {
        CC->State = CS_S5_CHALLENGE;
        return OnPAC_S5_Challenge(CC, DP, DS);
    }
    return InvalidDataSize;
}

void OnPAC_AuthResult(uint64_t ConnectionId, const xClientAuthResult * AR) {
    auto CC = GetClientConnectionById(ConnectionId);
    if (!CC) {
        DEBUG_LOG("timeout auth result");
        return;
    }

    do {
        auto OS = std::ostringstream();
        OS << "AuditId: " << AR->AuditId << " ";
        OS << "CountryId: " << AR->CountryId << " ";
        OS << "StateId: " << AR->StateId << " ";
        OS << "RequireIpv6: " << AR->RequireIpv6 << " ";
        OS << "RequireUdp: " << AR->RequireUdp << " ";
        OS << "AlwaysChangeIp: " << AR->AlwaysChangeIp << " ";
        OS << "PersistentDeviceBinding: " << AR->PersistentDeviceBinding << " ";
        OS << "PAToken: " << AR->PAToken << " ";
        OS << "ThirdRedirect: " << AR->ThirdRedirect << " ";
        DEBUG_LOG("%s", OS.str().c_str());
    } while (false);

    switch (CC->State) {
        case CS_S5_WAIT_FOR_AUTH_RESULT:
            OnPAC_S5_AuthResult(CC, AR);
            break;
        default:
            DeferKillClientConnection(CC);
    }
    return;
}

void OnPAC_DeviceSelectResult(uint64_t ConnectionId, const xDeviceSelectorResult & Result) {
    DEBUG_LOG("RID: %" PRIx64 ", RelayServerId=%" PRIx64 ", DeviceConnectionId=%" PRIx64 "", ConnectionId, Result.DeviceRelayServerRuntimeId, Result.DeviceRelaySideId);
    auto CC = GetClientConnectionById(ConnectionId);
    if (!CC) {
        DEBUG_LOG("Missing original connection");
        return;
    }

    switch (CC->State) {
        case CS_S5_WAIT_FOR_DEVICE_RESULT:
            OnPAC_S5_DeviceResult(CC, Result);
            break;

        default:
            DeferKillClientConnection(CC);
    }
}
