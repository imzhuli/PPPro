#pragma once
#include <pp_common/_.hpp>
//
#include "./device_connection_manager.hpp"
#include "./device_manager.hpp"
#include "./proxy_connection_manager.hpp"

class xDeviceRelayService
    : public xTcpServer::iListener
    , public xTcpConnection::iListener {
public:
    bool Init(xIoContext * CP, xNetAddress ControllerBindAddress, xNetAddress DataBindAddress, xNetAddress ProxyBindAddress);
    void Clean();
    void Tick(uint64_t NowMS);

    void OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
    void OnNewControlConnection(xSocket && NativeHandle);
    void OnNewDataConnection(xSocket && NativeHandle);
    void OnNewProxyConnection(xSocket && NativeHandle);

    void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
    void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
    size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

    bool OnTerminalInitCtrlStream(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);
    bool OnTerminalInitDataStream(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);
    bool OnTerminalTargetConnectionUpdate(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);
    bool OnTerminalPostData(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);
    bool OnTerminalDnsQueryResp(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);

    bool OnProxyChallenge(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize);
    bool OnProxyCreateConnection(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize);
    bool OnProxyDestroyConnection(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize);
    bool OnProxyPushData(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize);
    bool OnProxyDnsQuery(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize);
    bool OnProxyNotifyConnectionState(const ubyte * Payload, size_t PayloadSize);

protected:
    void RemoveDeviceFromConnection(xRD_DeviceConnection * Conn);
    void RemoveDevice(xDevice * Device);

    bool OnDataPacket(xRD_DeviceConnection * Conn, xPacketCommandId CommandId, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);
    bool OnCtrlPacket(xRD_DeviceConnection * Conn, xPacketCommandId CommandId, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);
    bool OnProxyPacket(xRD_ProxyConnection * Conn, xPacketCommandId CommandId, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize);

    //
    xIoContext * IoContext = nullptr;
    xTicker      Ticker;
    xTcpServer   ControlServer;
    xTcpServer   DataServer;
    xTcpServer   ProxyServer;
};
