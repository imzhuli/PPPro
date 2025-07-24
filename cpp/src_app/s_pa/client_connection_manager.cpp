#include "./client_connection_manager.hpp"

#include "../lib_server_util/service_common.hpp"

bool xPA_ClientConnectionManager::Init(xIoContext * ICP) {
    assert(ICP);
    this->ICP = ICP;
    if (!this->ConnectionIdPool.Init(MAX_CLIENT_CONNECTION_ID_COUNT)) {
        Reset(this->ICP);
        return false;
    }
    return true;
}

void xPA_ClientConnectionManager::Clean() {
    //
    while (auto P = UpCast(AuthConnectionList.PopHead())) {
        DoDeleteConnection(P);
    }
    while (auto P = UpCast(IdleConnectionList.PopHead())) {
        DoDeleteConnection(P);
    }
    while (auto P = UpCast(KillConnectionList.PopHead())) {
        DoDeleteConnection(P);
    }
    Reset(ICP);
}

auto xPA_ClientConnectionManager::GetConnectionById(uint64_t ConnectionId) -> xPA_ClientConnection * {
    auto PP = ConnectionIdPool.CheckAndGet(ConnectionId);
    if (!PP) {
        return nullptr;
    }
    return *PP;
}

void xPA_ClientConnectionManager::KeepAlive(xPA_ClientConnection * CP) {
    CP->LastActiveTimestampMS = Ticker();
    IdleConnectionList.GrabTail(*CP);
}

void xPA_ClientConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);

    auto AuthTimepoint = Ticker() - MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS;
    while (auto P = UpCast(AuthConnectionList.PopHead([AuthTimepoint](auto & C) { return C.LastActiveTimestampMS <= AuthTimepoint; }))) {
        DoDeleteConnection(P);
    }

    auto IdleTimepoint = Ticker() - MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS;
    while (auto P = UpCast(IdleConnectionList.PopHead([IdleTimepoint](auto & C) { return C.LastActiveTimestampMS <= IdleTimepoint; }))) {
        DoDeleteConnection(P);
    }

    while (auto P = UpCast(KillConnectionList.PopHead())) {
        DoDeleteConnection(P);
    }
}

void xPA_ClientConnectionManager::KillConnection(xPA_ClientConnection * CP) {
    KillConnectionList.GrabTail(*CP);
}

void xPA_ClientConnectionManager::DoCreateConnection(xSocket && NativeHandle) {
    auto CP = new (std::nothrow) xPA_ClientConnection;
    if (!CP) {
        return;
    }
    if (!CP->Init(ICP, std::move(NativeHandle), this)) {
        delete CP;
        return;
    }
    InitClientState(CP);

    auto Id = ConnectionIdPool.Acquire(CP);
    if (!Id) {
        CP->Clean();
        delete CP;
        return;
    }

    CP->Owner        = this;
    CP->ConnectionId = Id;

    CP->LastActiveTimestampMS = Ticker();
    AuthConnectionList.AddTail(*CP);

    Logger->I("Create new connection: id=%" PRIu64 "", CP->ConnectionId);
}

void xPA_ClientConnectionManager::DoDeleteConnection(xPA_ClientConnection * CP) {
    Logger->I("Delete connection: id=%" PRIu64 "", CP->ConnectionId);
    assert(ConnectionIdPool.Check(CP->ConnectionId) && ConnectionIdPool[CP->ConnectionId] == CP);
    assert(CP->IsOpen());

    ScheduleClientStateChange(CP, CloseClientStateHandler);
    FinalClientState(CP);

    CP->Clean();
    ConnectionIdPool.Release(CP->ConnectionId);
    delete CP;
}

void xPA_ClientConnectionManager::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
    DoCreateConnection(std::move(NativeHandle));
}

void xPA_ClientConnectionManager::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    auto CP = static_cast<xPA_ClientConnection *>(TcpConnectionPtr);
    KillConnection(CP);
}

size_t xPA_ClientConnectionManager::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    DEBUG_LOG("OnData: \n%s", HexShow(DataPtr, DataSize).c_str());
    auto Client = static_cast<xPA_ClientConnection *>(TcpConnectionPtr);

    size_t TotalConsumed = 0;
    while (true) {
        auto Consumed = GetClientStateHandler(Client)->OnDataEvent(Client, DataPtr, DataSize);
        auto Updated  = UpdateClientState(Client);

        if (Consumed == InvalidDataSize) {
            return InvalidDataSize;
        }
        if (Consumed == 0 && !Updated) {
            return TotalConsumed;
        }

        DataPtr       += Consumed;
        DataSize      -= Consumed;
        TotalConsumed += Consumed;
    }
}

void xPA_ClientConnectionManager::OnAuthResult(uint64_t RequestContextId, const xClientAuthResult & AuthResult) {
    DEBUG_LOG("");
}
