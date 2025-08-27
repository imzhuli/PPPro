#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/dns_query.hpp>
#include <pp_protocol/device_relay/init_ctrl_stream.hpp>
#include <pp_protocol/device_relay/init_data_stream.hpp>
#include <pp_protocol/device_relay/internal_key.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>

bool xDeviceRelayService::OnCtrlPacket(xRD_DeviceConnection * Conn, xPacketCommandId CommandId, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    DEBUG_LOG("Cmd=%" PRIx64 ", Request body: \n%s", CommandId, HexShow(Payload, PayloadSize).c_str());
    switch (CommandId) {
        case Cmd_DV_RL_InitCtrlStream: {
            return OnTerminalInitCtrlStream(Conn, RequestId, Payload, PayloadSize);
        }
        case Cmd_DV_RL_DnsQueryResp: {
            return OnTerminalDnsQueryResp(Conn, RequestId, Payload, PayloadSize);
        }
    }
    return false;
}

bool xDeviceRelayService::OnTerminalInitCtrlStream(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    auto S = xInitCtrlStream();
    if (!S.Deserialize(Payload, PayloadSize)) {
        return false;
    }
    // TODO: check
    DEBUG_LOG("New Terminal Device: Ipv4:%s, Ipv6:%s", S.Ipv4Address.IpToString().c_str(), S.Ipv6Address.IpToString().c_str());

    auto R       = xInitCtrlStreamResp();
    R.DeviceId   = 0;
    R.CtrlId     = Conn->ConnectionId;
    R.DeviceKey  = MakeInternalKey(S.Ipv4Address, S.Ipv6Address, S.CheckKey);
    R.EnableIpv6 = S.Resolved3rdIpv6 || S.Ipv6Address;
    Conn->PostMessage(Cmd_DV_RL_InitCtrlStreamResp, RequestId, R);
    DeviceConnectionManager.KeepAlive(Conn);
    return true;
}

bool xDeviceRelayService::OnTerminalDnsQueryResp(xRD_DeviceConnection * Conn, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    auto Resp = xTR_DnsQueryResp();
    //
    DEBUG_LOG("DnsQueryResp: \n%s", HexShow(Payload, PayloadSize).c_str());

    if (!Conn->IsType_Ctrl() || !Resp.Deserialize(Payload, PayloadSize)) {
        return false;
    }
    DEBUG_LOG(
        "New Terminal Device: Hostname:%s Ipv4:%s, Ipv6:%s", std::string(Resp.HostnameView).c_str(), Resp.PrimaryIpv4.IpToString().c_str(), Resp.PrimaryIpv6.IpToString().c_str()
    );

    auto RCC = RelayConnectionManager.GetConnectionById(RequestId);
    if (!RCC) {
        DEBUG_LOG("Connection not found");
        return true;
    }

    auto CC               = xTR_CreateConnection();
    CC.RelaySideContextId = RCC->RelaySideContextId;
    if (Resp.PrimaryIpv6 && Resp.PrimaryIpv6.SA6[0]) {
        DEBUG_LOG("using ipv6");
        CC.TargetAddress = Resp.PrimaryIpv6;
    } else if (Resp.PrimaryIpv4 && Resp.PrimaryIpv4.SA4[0]) {
        DEBUG_LOG("using ipv4");
        CC.TargetAddress = Resp.PrimaryIpv4;
    } else {
        // TODO: Notify and destroy connection
        X_PERROR("TODO: Destroy and notify dns failure");
        return true;
    }

    CC.TargetAddress.Port = RCC->TargetPort;
    DEBUG_LOG("Select target address: %s", CC.TargetAddress.ToString().c_str());

    Conn->PostMessage(Cmd_DV_RL_CreateConnection, 0, CC);
    return true;
}
