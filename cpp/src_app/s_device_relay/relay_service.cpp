#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/init_ctrl_stream.hpp>
#include <pp_protocol/device_relay/init_data_stream.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>

bool xDeviceRelayService::Init(xIoContext * CP, xNetAddress ControllerBindAddress, xNetAddress DataBindAddress, xNetAddress ProxyBindAddress) {
    RuntimeAssert(ControlServer.Init(CP, ControllerBindAddress, this));
    RuntimeAssert(DataServer.Init(CP, DataBindAddress, this));
    RuntimeAssert(ProxyServer.Init(CP, ProxyBindAddress, this));
    return true;
}

void xDeviceRelayService::Clean() {
    ControlServer.Clean();
    DataServer.Clean();
    ProxyServer.Clean();
}

void xDeviceRelayService::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
}

void xDeviceRelayService::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
    if (TcpServerPtr == &ControlServer) {
        return OnNewControlConnection(std::move(NativeHandle));
    } else if (TcpServerPtr == &DataServer) {
        return OnNewDataConnection(std::move(NativeHandle));
    } else if (TcpServerPtr == &ProxyServer) {
        return OnNewProxyConnection(std::move(NativeHandle));
    }
    Unreachable();
}

void xDeviceRelayService::OnNewControlConnection(xSocket && NativeHandle) {
    auto Conn = DeviceConnectionManager.AcceptConnection(std::move(NativeHandle), this);
    if (!Conn) {
        return;
    }
    Conn->SetType_Ctrl();
}

void xDeviceRelayService::OnNewDataConnection(xSocket && NativeHandle) {
    auto Conn = DeviceConnectionManager.AcceptConnection(std::move(NativeHandle), this);
    if (!Conn) {
        return;
    }
    Conn->SetType_Data();
}

void xDeviceRelayService::OnNewProxyConnection(xSocket && NativeHandle) {
    X_DEBUG_PRINTF("");
    auto Conn = ProxyConnectionManager.AcceptConnection(std::move(NativeHandle), this);
    if (!Conn) {
        X_DEBUG_PRINTF("");
        return;
    }
    Conn->SetType_ProxyClient();
}

void xDeviceRelayService::OnConnected(xTcpConnection * TcpConnectionPtr) {
    Fatal("should not be called");
}

void xDeviceRelayService::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
    auto Conn = static_cast<xRD_ConnectionBase *>(TcpConnectionPtr);
    if (Conn->IsType_Ctrl()) {
        auto DC = static_cast<xRD_DeviceConnection *>(Conn);
        RemoveDeviceFromConnection(DC);
        DeviceConnectionManager.DeferReleaseConnection(DC);
        return;
    } else if (Conn->IsType_Data()) {
        auto DC = static_cast<xRD_DeviceConnection *>(Conn);
        DeviceConnectionManager.DeferReleaseConnection(DC);
        return;
    }
    assert(Conn->IsType_ProxyClient());
    auto PC = static_cast<xRD_ProxyConnection *>(Conn);
    ProxyConnectionManager.DeferReleaseConnection(PC);
}

size_t xDeviceRelayService::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
    auto   Conn       = static_cast<xRD_ConnectionBase *>(TcpConnectionPtr);
    size_t RemainSize = DataSize;
    while (RemainSize >= PacketHeaderSize) {
        auto Header = xPacketHeader::Parse(DataPtr);
        if (!Header) { /* header error */
            return InvalidDataSize;
        }
        auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
        if (RemainSize < PacketSize) {        // wait for data
            break;
        }
        if (Header.IsRequestKeepAlive()) {
            Conn->PostKeepAlive();
            if (Conn->IsType_Ctrl()) {
                auto PDC = static_cast<xRD_DeviceConnection *>(Conn);
                DeviceConnectionManager.KeepAlive(PDC);
                if (PDC->DeviceId) {
                    X_DEBUG_PRINTF("DeviceCtrl:KeepAlive %" PRIx64 "", PDC->DeviceId);
                    DeviceManager.ReportDeviceOnState(PDC->DeviceId);
                }
            } else if (Conn->IsType_Data()) {
                auto PDC = static_cast<xRD_DeviceConnection *>(Conn);
                DeviceConnectionManager.KeepAlive(PDC);
                if (PDC->DeviceId) {
                    X_DEBUG_PRINTF("DeviceData:KeepAlive %" PRIx64 "", PDC->DeviceId);
                }
            } else {
                assert(Conn->IsType_ProxyClient());
                ProxyConnectionManager.KeepAlive(static_cast<xRD_ProxyConnection *>(Conn));
            }
        } else {
            auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
            auto PayloadSize = Header.GetPayloadSize();
            // dispatch packet
            if (Conn->IsType_Ctrl()) {
                if (!OnCtrlPacket(static_cast<xRD_DeviceConnection *>(Conn), Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
                    return InvalidDataSize;
                }
            } else if (Conn->IsType_Data()) {
                if (!OnDataPacket(static_cast<xRD_DeviceConnection *>(Conn), Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
                    return InvalidDataSize;
                }
            } else {
                assert(Conn->IsType_ProxyClient());
                if (!OnProxyPacket(static_cast<xRD_ProxyConnection *>(Conn), Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
                    X_DEBUG_PRINTF("?? ");
                    return InvalidDataSize;
                }
            }
        }
        DataPtr    += PacketSize;
        RemainSize -= PacketSize;
    }
    return DataSize - RemainSize;
}

void xDeviceRelayService::RemoveDeviceFromConnection(xRD_DeviceConnection * Conn) {
    auto DeviceId = Conn->DeviceId;
    auto Device   = DeviceManager.GetDeviceById(DeviceId);
    if (!Device) {
        return;
    }
    DeviceManager.ReportDeviceOfflineState(Device);
    DeviceManager.ReleaseDevice(Device);
}

void xDeviceRelayService::RemoveDevice(xDevice * Device) {
    if (Device->CtrlConnection) {
        DeviceConnectionManager.DeferReleaseConnection(Steal(Device->CtrlConnection));
    }
    if (Device->DataConnection) {
        DeviceConnectionManager.DeferReleaseConnection(Steal(Device->DataConnection));
    }
    DeviceManager.ReleaseDevice(Device);
}
