#include "./proxy_connection_manager.hpp"

static constexpr const uint64_t CHALLENGE_TIMEOUT = 2'000;
static constexpr const uint64_t IDLE_TIMEOUT      = 3 * 60'000;

bool xRD_ProxyConnectionManager::Init(xIoContext * ICP, size32_t MaxConnections) {
    RuntimeAssert(this->ICP = ICP);
    RuntimeAssert(ConnectionIdManager.Init(MaxConnections));
    Ticker.Update();
    return true;
}

void xRD_ProxyConnectionManager::Clean() {
    Todo("delete all connections");
    ConnectionIdManager.Clean();
    this->ICP = nullptr;
}

xRD_ProxyConnection * xRD_ProxyConnectionManager::AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener) {
    return CreateConnection(std::move(NativeHandle), Listener);
}

void xRD_ProxyConnectionManager::KeepAlive(xRD_ProxyConnection * Conn) {
    if (Conn->HasMark_Delete()) {
        return;
    }
    if (!Conn->ChallengeReady) {
        X_DEBUG_PRINTF("proxy conn not ready");
        return;
    }
    Conn->IdleTimestamMS = Ticker();
    IdleConnectionList.GrabTail(*Conn);
}

void xRD_ProxyConnectionManager::DeferReleaseConnection(xRD_ProxyConnection * Conn) {
    if (Conn->HasMark_Delete()) {
        return;
    }
    Conn->Mark_Delete();
    KillConnectionList.GrabTail(*Conn);
}

void xRD_ProxyConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    RemoveChallengeTimeoutConnections();
    RemoveIdleConnections();

    while (auto C = KillConnectionList.PopHead()) {
        DestroyConnection(C);
    }
}

void xRD_ProxyConnectionManager::RemoveChallengeTimeoutConnections() {
    auto Timepoint = Ticker() - CHALLENGE_TIMEOUT;
    while (auto C = ChallengeConnectionList.PopHead([Timepoint](const xRD_ProxyConnection & PC) { return PC.IdleTimestamMS <= Timepoint; })) {
        X_DEBUG_PRINTF("ProxyConnId:%" PRIx64 ", Timeout=%" PRIu64 "", C->ConnectionId, C->IdleTimestamMS - Timepoint);
        DeferReleaseConnection(C);
    };
}

void xRD_ProxyConnectionManager::RemoveIdleConnections() {
    auto Timepoint = Ticker() - IDLE_TIMEOUT;
    while (auto C = IdleConnectionList.PopHead([Timepoint](const xRD_ProxyConnection & PC) { return PC.IdleTimestamMS <= Timepoint; })) {
        DeferReleaseConnection(C);
    };
}

xRD_ProxyConnection * xRD_ProxyConnectionManager::CreateConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener) {
    auto C = new (std::nothrow) xRD_ProxyConnection();
    if (!C) {
        XelCloseSocket(NativeHandle);
        return nullptr;
    }
    auto Id = ConnectionIdManager.Acquire(C);
    if (!Id) {
        delete C;
        XelCloseSocket(NativeHandle);
        return nullptr;
    }
    if (!C->Init(ICP, std::move(NativeHandle), Listener)) {
        ConnectionIdManager.Release(Id);
        delete C;
        return nullptr;
    }

    C->ConnectionId   = Id;
    C->IdleTimestamMS = Ticker();
    ChallengeConnectionList.AddTail(*C);
    return C;
}

void xRD_ProxyConnectionManager::DestroyConnection(xRD_ProxyConnection * Conn) {
    auto Id = Conn->ConnectionId;
    ConnectionIdManager.Release(Id);
    Conn->Clean();
    delete Conn;
}

void xRD_ProxyConnectionManager::FreeAllConnections() {
    KillConnectionList.GrabListTail(ChallengeConnectionList);
    KillConnectionList.GrabListTail(IdleConnectionList);
    while (auto C = KillConnectionList.PopHead()) {
        DestroyConnection(C);
    }
}
