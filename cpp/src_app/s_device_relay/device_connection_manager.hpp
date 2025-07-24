#pragma once
#include "./connection.hpp"

class xRD_DeviceConnection
    : public xRD_ConnectionBase
    , public xListNode {

public:
    uint64_t DeviceId = 0;
    //
};

class xRD_DeviceConnectionManager {

public:
    bool Init(xIoContext * ICP, size32_t MaxRD_DeviceConnections = 50'0000);
    void Clean();

    void Tick(uint64_t NowMS);
    void RemoveIdleConnections();

    xRD_DeviceConnection * AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener);
    void                   DeferReleaseConnection(xRD_DeviceConnection * Conn);
    void                   KeepAlive(xRD_DeviceConnection * Conn);

    xRD_DeviceConnection * GetConnectionById(uint64_t ConnectionId) {
        auto PR = ConnectionIdManager.CheckAndGet(ConnectionId);
        if (!PR) {
            return nullptr;
        }
        return *PR;
    }

private:
    xRD_DeviceConnection * CreateConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener);
    void                   DestroyConnection(xRD_DeviceConnection * Conn);
    void                   FreeAllConnections();

private:
    xIoContext *                            ICP;
    xTicker                                 Ticker;
    uint64_t                                IdleTimeoutMS = 120'000;
    xList<xRD_DeviceConnection>             NewConnectionList;
    xList<xRD_DeviceConnection>             IdleConnectionList;
    xList<xRD_DeviceConnection>             SlowKillConnectionList;
    xList<xRD_DeviceConnection>             KillConnectionList;
    xIndexedStorage<xRD_DeviceConnection *> ConnectionIdManager;
};
