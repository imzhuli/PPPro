#include "./client_manager.hpp"

#include "./_global.hpp"

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
static xel::xIndexedStorage<xPA_ClientConnection *> ClientIdManager;
static xPA_ClientConnectionListenerWrapper          ClientConnectionListenerWrapper;
static xClientServerWrapper                         ClientServerWrapper;

static xList<xPA_ClientConnectionIdleNode> AuthTimeoutList;
static xList<xPA_ClientConnectionIdleNode> ClientIdleList;
static xList<xPA_ClientConnectionKillNode> ClientPassiveKillList;
static xList<xPA_ClientConnectionKillNode> ClientKillList;

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
    if (CC->State.M == ePA_ClientMainState::F) {
        return;
    }
    CC->State.M = ePA_ClientMainState::F;
    if (!CC->HasPendingWrites()) {
        DeferKillClientConnection(CC);
        return;
    }
    CC->KillingScheduledTimestampMS = ServiceTicker();
    ClientPassiveKillList.GrabTail(*CC);
}

std::string BuildIpAuthInfo(xPA_ClientConnection * ClientConnection) {
    if (ConfigExportTcpBindAddress.empty()) {
        return {};
    }
    auto ClientAddress = ClientConnection->Conn.GetRemoteAddress();
    if (!ClientAddress) {
        return {};
    }
    auto IpAuthString = "P_" + ConfigExportTcpBindAddress + "_" + ClientAddress.IpToString();
    DEBUG_LOG("IpAuthString=%s", IpAuthString.c_str());
    return IpAuthString;
}

////////////////

void OnPAClientConnectionAccepted(xPA_ClientConnection * CC) {
    CC->LastActivityTimestampMS = ServiceTicker();
    AuthTimeoutList.AddTail(*CC);
}

void OnPAClientConnectionPeerClose(xPA_ClientConnection * CC) {
    switch (CC->State.M) {
            // case CS_S_READY:  // pass-through
            // case CS_H_READY:  // pass-through
            // case CS_T_READY:  // pass-through
            //     RequestRelayCloseConnection(CC->ConnectionId, CC->DeviceRelayServerRuntimeId, CC->RelaySideContextId);
            //     break;

        default:
            DEBUG_LOG("unhandled state on client close");
            DeferKillClientConnection(CC);
            break;
    }
}

void OnPAClientConnectionFlush(xPA_ClientConnection * CC) {
    if (CC->State.M == ePA_ClientMainState::F) {
        DEBUG_LOG("close on flush");
        DeferKillClientConnection(CC);
    }
}

size_t OnPAClientConnectionData(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    switch (CC->State.M) {
        case ePA_ClientMainState::C:
            return OnPAC_Challenge(CC, DP, DS);
            // case CS_S_CHALLENGE:
            //     return OnPAC_S_Challenge(CC, DP, DS);
            // case CS_S_WAIT_FOR_AUTH_INFO:
            //     return OnPAC_S_AuthInfo(CC, DP, DS);
            // case CS_S_WAIT_FOR_TARGET_ADDRESS:
            //     return OnPAC_S_TargetAddress(CC, DP, DS);
            // case CS_S_READY:
            //     return OnPAC_S_UploadTcpData(CC, DP, DS);

            // case CS_H_CHALLENGE:
            //     return OnPAC_H_Challenge(CC, DP, DS);
            // case CS_H_READY:
            //     return OnPAC_H_UploadData(CC, DP, DS);

            // case CS_T_CHALLENGE:
            //     return OnPAC_T_Challenge(CC, DP, DS);
            // case CS_T_READY:
            //     return OnPAC_T_UploadData(CC, DP, DS);

        default:
            DEBUG_LOG("Unhandled state: %u", (unsigned)CC->State.M);
            return InvalidDataSize;
    }
    return DS;
}

/////////////////////

size_t OnPAC_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    if (DP[0] == '\x05') {
        CC->State.M = ePA_ClientMainState::S;
        CC->State.S = ePA_ClientSubState::_;
        return OnPAC_S_Challenge(CC, DP, DS);
    }
    if (DP[0] == 'C') {
        CC->State.M = ePA_ClientMainState::T;
        CC->State.S = ePA_ClientSubState::_;
        return OnPAC_T_Challenge(CC, DP, DS);
    }
    // currently only HTTP normal is accepted:
    CC->State.M = ePA_ClientMainState::H;
    CC->State.S = ePA_ClientSubState::_;
    return OnPAC_H_Challenge(CC, DP, DS);
}

void OnPAC_AuthResult(uint64_t ConnectionId, const xClientAuthResult * AR) {
    auto CC = GetClientConnectionById(ConnectionId);
    if (!CC) {
        DEBUG_LOG("timeout auth result");
        return;
    }

    switch (CC->State.M) {
        case ePA_ClientMainState::S:
            OnPAC_S_AuthResult(CC, AR);
            break;
        case ePA_ClientMainState::H:
            OnPAC_H_AuthResult(CC, AR);
            break;
        case ePA_ClientMainState::T:
            OnPAC_T_AuthResult(CC, AR);
            break;
        default:
            DeferKillClientConnection(CC);
            break;
    }
    return;
}

void OnPAC_DeviceSelectResult(uint64_t ConnectionId, const xDeviceSelectorResult & Result) {
    DEBUG_LOG("RID: %" PRIx64 ", RelayServerId=%" PRIx64 ", DeviceConnectionId=%" PRIx64 "", ConnectionId, Result.DeviceRelayServerRuntimeId, Result.DeviceRelaySideId);
    // auto CC = GetClientConnectionById(ConnectionId);
    // if (!CC) {
    //     DEBUG_LOG("Missing original connection");
    //     return;
    // }

    // switch (CC->State) {
    //     case CS_S_WAIT_FOR_DEVICE_RESULT:
    //         OnPAC_S_DeviceResult(CC, Result);
    //         break;
    //     case CS_H_WAIT_FOR_DEVICE_RESULT:
    //         OnPAC_H_DeviceResult(CC, Result);
    //         break;
    //     case CS_T_WAIT_FOR_DEVICE_RESULT:
    //         OnPAC_T_DeviceResult(CC, Result);
    //         break;

    //     default:
    //         DeferKillClientConnection(CC);
    // }
}
