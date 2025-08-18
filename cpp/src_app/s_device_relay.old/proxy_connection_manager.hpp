#pragma once
#include "./connection.hpp"

#include <pp_common/_.hpp>

class xRD_ProxyConnection;
class xRD_ProxyConnectionManager;

class xRD_ProxyConnection
    : public xRD_ConnectionBase
    , public xListNode {

public:
    void SetChallengeReady() {
        ChallengeReady = true;
    }

private:
    bool ChallengeReady = false;
    //
    friend class xRD_ProxyConnectionManager;
};

class xRD_ProxyConnectionManager {
public:
    bool Init(xIoContext * ICP, size32_t MaxRD_DeviceConnections);
    void Clean();

    void Tick(uint64_t NowMS);
    void RemoveChallengeTimeoutConnections();
    void RemoveIdleConnections();

    xRD_ProxyConnection * AcceptConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener);
    void                  DeferReleaseConnection(xRD_ProxyConnection * Conn);
    void                  KeepAlive(xRD_ProxyConnection * Conn);

    xRD_ProxyConnection * GetConnectionById(uint64_t ConnectionId) {
        auto PR = ConnectionIdManager.CheckAndGet(ConnectionId);
        if (!PR) {
            return nullptr;
        }
        return *PR;
    }

private:
    xRD_ProxyConnection * CreateConnection(xSocket && NativeHandle, xTcpConnection::iListener * Listener);
    void                  DestroyConnection(xRD_ProxyConnection * Conn);
    void                  FreeAllConnections();

protected:
    xIoContext *                           ICP;
    xTicker                                Ticker;
    xList<xRD_ProxyConnection>             ChallengeConnectionList;
    xList<xRD_ProxyConnection>             IdleConnectionList;
    xList<xRD_ProxyConnection>             KillConnectionList;
    xIndexedStorage<xRD_ProxyConnection *> ConnectionIdManager;
};
