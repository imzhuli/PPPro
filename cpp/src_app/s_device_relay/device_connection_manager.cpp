#include "./device_connection_manager.hpp"

/***************
 * Connection manager
 */

bool xRD_DeviceConnectionManager::Init(xIoContext * ICP, size32_t MaxRD_DeviceConnections) {
    RuntimeAssert(this->ICP = ICP);
    RuntimeAssert(ConnectionIdManager.Init(MaxRD_DeviceConnections));
    Ticker.Update();
    return true;
}

void xRD_DeviceConnectionManager::Clean() {
    FreeAllConnections();
    ConnectionIdManager.Clean();
    this->ICP = nullptr;
}

void xRD_DeviceConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    RemoveIdleConnections();
}

void xRD_DeviceConnectionManager::RemoveIdleConnections() {
    auto NewConnectionKillTimepoint = Ticker() - 3'000;
    while (auto TOC = NewConnectionList.PopHead([NewConnectionKillTimepoint](const xRD_DeviceConnection & C) -> bool {
        return C.IdleTimestamMS <= NewConnectionKillTimepoint;
    })) {
        X_DEBUG_PRINTF("New Connection: %" PRIx64 "", TOC->ConnectionId);
        KillConnectionList.GrabTail(*TOC);
    }

    auto KillTimepoint = Ticker() - IdleTimeoutMS;
    while (auto TOC = IdleConnectionList.PopHead([KillTimepoint](const xRD_DeviceConnection & C) -> bool { return C.IdleTimestamMS <= KillTimepoint; })) {
        KillConnectionList.GrabTail(*TOC);
    }

    while (auto TOC = KillConnectionList.PopHead()) {
        DestroyConnection(TOC);
    }
}

xRD_DeviceConnection * xRD_DeviceConnectionManager::AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener) {
    return CreateConnection(std::move(NativeHandle), Listener);
}

xRD_DeviceConnection * xRD_DeviceConnectionManager::CreateConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener) {
    auto C = new (std::nothrow) xRD_DeviceConnection();
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
    NewConnectionList.AddTail(*C);
    return C;
}

void xRD_DeviceConnectionManager::DeferReleaseConnection(xRD_DeviceConnection * Conn) {
    if (Conn->HasMark_Delete()) {
        return;
    }
    Conn->Mark_Delete();
    KillConnectionList.GrabTail(*Conn);
}

void xRD_DeviceConnectionManager::KeepAlive(xRD_DeviceConnection * Conn) {
    if (Conn->HasMark_Delete()) {
        return;
    }
    Conn->IdleTimestamMS = Ticker();
    IdleConnectionList.GrabTail(*Conn);
}

void xRD_DeviceConnectionManager::DestroyConnection(xRD_DeviceConnection * Conn) {
    assert(Conn->ConnectionId);
    X_DEBUG_PRINTF("DestroyConnection: %" PRIx64 "", Conn->ConnectionId);
    RuntimeAssert(ConnectionIdManager.CheckAndRelease(Conn->ConnectionId));
    Conn->Clean();
    delete Conn;
}

void xRD_DeviceConnectionManager::FreeAllConnections() {
    KillConnectionList.GrabListTail(NewConnectionList);
    KillConnectionList.GrabListTail(IdleConnectionList);
    KillConnectionList.GrabListTail(SlowKillConnectionList);
    while (auto PC = KillConnectionList.PopHead()) {
        DestroyConnection(PC);
    }
}
