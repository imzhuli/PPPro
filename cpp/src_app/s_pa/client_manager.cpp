#include "./client_manager.hpp"

#include "_global.hpp"

static constexpr const uint64_t MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS         = 3'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_PASSIVE_KILL_TIMEOUT_MS = 1'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS         = 90'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_ID_COUNT                = 100'000;

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

static xTcpServer                          ClientTcpServer;
static xPA_ClientConnectionListenerWrapper ClientConnectionListenerWrapper;
static xClientServerWrapper                ClientServerWrapper;
static xList<xPA_ClientConnectionIdleNode> AuthTimeoutList;
static xList<xPA_ClientConnectionKillNode> ClientPassiveKillList;
static xList<xPA_ClientConnectionKillNode> ClientKillList;

//////////////// public functions

void InitClientManager() {
    RuntimeAssert(ClientTcpServer.Init(ServiceIoContext, ConfigTcpBindAddress, &ClientServerWrapper));
}

void CleanClietManager() {
    ClientTcpServer.Clean();
}

void TickClientManager(uint64_t NowMS) {
    auto AuthTimeoutPred = [KillPoint = NowMS - MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS](const xPA_ClientConnectionIdleNode & N) { return N.LastActivityTimestampMS <= KillPoint; };
    while (auto P = static_cast<xPA_ClientConnection *>(AuthTimeoutList.PopHead(AuthTimeoutPred))) {
        DestroyClientConnection(P);
    }

    auto PassiveTimeoutPred = [KillPoint = NowMS - MAX_CLIENT_CONNECTION_PASSIVE_KILL_TIMEOUT_MS](const xPA_ClientConnectionKillNode & N) {
        return N.KillingScheduledTimestampMS <= KillPoint;
    };
    while (auto P = static_cast<xPA_ClientConnection *>(ClientPassiveKillList.PopHead(PassiveTimeoutPred))) {
        DestroyClientConnection(P);
    }

    while (auto P = static_cast<xPA_ClientConnection *>(ClientKillList.PopHead())) {
        DestroyClientConnection(P);
    }
}

void AsyncRequireAuthInfo(uint64_t ClientConnectionId, const std::string_view & AuthView) {
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
    return C;
}

void DestroyClientConnection(xPA_ClientConnection * CC) {
    CC->Conn.Clean();
    delete CC;
}

void DeferKillClientConnection(xPA_ClientConnection * CC) {
    ClientKillList.GrabTail(*CC);
}

void SchedulePassiveKillClientConnection(xPA_ClientConnection * CC) {
    CC->KillingScheduledTimestampMS = ServiceTicker();
    ClientPassiveKillList.GrabTail(*CC);
}

void OnPAClientConnectionAccepted(xPA_ClientConnection * CC) {
    CC->LastActivityTimestampMS = ServiceTicker();
    AuthTimeoutList.AddTail(*CC);
}

void OnPAClientConnectionPeerClose(xPA_ClientConnection * CC) {
    DeferKillClientConnection(CC);
}

void OnPAClientConnectionFlush(xPA_ClientConnection * CC) {
    if (CC->State == CS_KILL_ON_FLUSH) {
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
        case CS_S5_WAIT_FOR_IP_WHITELIST:
            return InvalidDataSize;
        case CS_S5_WAIT_FOR_TARGET_ADDRESS:
            return OnPAC_S5_TargetAddress(CC, DP, DS);
        case CS_S5_WAIT_FOR_CONECTION_ESTABLISH:
            return InvalidDataSize;

        default:
            DEBUG_LOG("Unhandled state");
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