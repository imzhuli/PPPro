#include "./relay_service.hpp"

#include "./_global.hpp"
#include "./connection.hpp"

#include <algorithm>
#include <pp_protocol/command.hpp>
#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/dns_query.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/proxy_relay/challenge.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>

bool xDeviceRelayService::OnProxyPacket(xRD_ProxyConnection * Conn, xPacketCommandId CommandId, xPacketRequestId RequestId, const ubyte * Payload, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_PA_RL_Challenge:
            return OnProxyChallenge(Conn, Payload, PayloadSize);
        case Cmd_PA_RL_CreateConnection:
            return OnProxyCreateConnection(Conn, Payload, PayloadSize);
        case Cmd_PA_RL_DestroyConnection:
            return OnProxyDestroyConnection(Conn, Payload, PayloadSize);
        case Cmd_PA_RL_PostData:
            return OnProxyPushData(Conn, Payload, PayloadSize);
        case Cmd_PA_RL_NotifyConnectionState:
            return OnProxyNotifyConnectionState(Payload, PayloadSize);
        default:
            X_DEBUG_PRINTF("unrecognized protocol %" PRIx32 "", CommandId);
            break;
    }
    return false;
}

bool xDeviceRelayService::OnProxyChallenge(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize) {
    auto R = xPR_Challenge();
    if (!R.Deserialize(Payload, PayloadSize)) {
        return false;
    }
    X_DEBUG_PRINTF("");
    Conn->SetChallengeReady();
    ProxyConnectionManager.KeepAlive(Conn);

    auto Resp     = xPR_ChallengeResp();
    Resp.Accepted = true;
    Conn->PostMessage(Cmd_PA_RL_ChallengeResp, 0, Resp);

    return true;
}

bool xDeviceRelayService::OnProxyCreateConnection(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize) {
    auto R = xPR_CreateConnection();
    if (!R.Deserialize(Payload, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF("NewConnection: did=%" PRIx64 ", ProxySideConnectionId=%" PRIx64 "", R.RelaySideDeviceId, R.ProxySideConnectionId);

    auto D = DeviceManager.GetDeviceById(R.RelaySideDeviceId);
    if (!D) {
        X_DEBUG_PRINTF("Device not found");
        auto Resp                  = xPR_DestroyConnection();
        Resp.ProxySideConnectionId = R.ProxySideConnectionId;
        Conn->PostMessage(Cmd_PA_RL_DestroyConnection, 0, Resp);
        return true;
    }
    assert(D->CtrlConnection);

    auto RCC = RelayConnectionManager.Create();
    if (!RCC) {
        auto F                  = xPR_ConnectionStateNotify();
        F.NewState              = xPR_ConnectionStateNotify::STATE_CLOSED;
        F.ProxySideConnectionId = R.ProxySideConnectionId;
        Conn->PostMessage(Cmd_PA_RL_NotifyConnectionState, 0, F);
        return true;
    }

    RCC->DeviceId              = D->DeviceRuntimeId;
    RCC->ProxyConnectionId     = Conn->ConnectionId;
    RCC->ProxySideConnectionId = R.ProxySideConnectionId;
    RCC->TargetPort            = R.HostnamePort;

    X_DEBUG_PRINTF("RelaySideConnectionId=%" PRIx64 ", TargetPort=%u", RCC->RelaySideConnectionId, (unsigned)RCC->TargetPort);

    if (R.TargetAddress) {
        auto CC                  = xTR_CreateConnection();
        CC.RelaySideConnectionId = RCC->RelaySideConnectionId;
        CC.TargetAddress         = R.TargetAddress;
        D->CtrlConnection->PostMessage(Cmd_DV_RL_CreateConnection, 0, CC);
    } else if (R.HostnameView.size()) {
        auto DQ         = xTR_DnsQuery();
        DQ.HostnameView = R.HostnameView;
        D->CtrlConnection->PostMessage(Cmd_DV_RL_DnsQuery, RCC->RelaySideConnectionId, DQ);
    } else {
        X_DEBUG_PRINTF("Invalid create connection request");
        return false;
    }

    return true;
}

bool xDeviceRelayService::OnProxyDestroyConnection(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize) {
    auto R = xPR_DestroyConnection();
    if (!R.Deserialize(Payload, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF("RelaySizeConnectionId=%" PRIx64 "", R.RelaySideConnectionId);
    auto RC = RelayConnectionManager.GetConnectionById(R.RelaySideConnectionId);
    if (!RC) {
        X_DEBUG_PRINTF("relay connection not found");
        return true;
    }

    auto RL                   = xTR_DestroyConnection();
    RL.DeviceSideConnectionId = RC->DeviceSideConnectionId;
    RL.RelaySideConnectionId  = RC->RelaySideConnectionId;

    auto D = DeviceManager.GetDeviceById(RC->DeviceId);
    if (!D) {
        X_DEBUG_PRINTF("Device not found");
        return true;
    }
    assert(D->DataConnection);

    D->DataConnection->PostMessage(Cmd_DV_RL_DestroyConnection, 0, RL);
    return true;
}

bool xDeviceRelayService::OnProxyPushData(xRD_ProxyConnection * Conn, const ubyte * Payload, size_t PayloadSize) {
    X_DEBUG_PRINTF("");
    auto R = xPR_PushData();
    if (!R.Deserialize(Payload, PayloadSize)) {
        X_DEBUG_PRINTF("invalid protocol");
        return false;
    }
    X_DEBUG_PRINTF(
        "RelaySideConnectionId=%" PRIx64 ", ProxySideConnectionId=%" PRIx64 ", Data=\n%s", R.RelaySideConnectionId, R.ProxySideConnectionId, HexShow(R.PayloadView).c_str()
    );

    auto RC = RelayConnectionManager.GetConnectionById(R.RelaySideConnectionId);
    if (!RC) {
        X_DEBUG_PRINTF("Connection not found");
        return true;
    }
    if (RC->ProxySideConnectionId != R.ProxySideConnectionId) {
        X_DEBUG_PRINTF("Proxy side connection id not match: %" PRIx64 ", %" PRIx64 "", RC->ProxySideConnectionId, R.ProxySideConnectionId);
        return true;
    }

    auto DC = DeviceManager.GetDeviceById(RC->DeviceId);
    if (!DC) {
        X_DEBUG_PRINTF("Device connection not found");
        return true;
    }

    auto Push                   = xTR_PostData();
    Push.DeviceSideConnectionId = RC->DeviceSideConnectionId;
    Push.RelaySideConnectionId  = RC->RelaySideConnectionId;
    Push.PayloadView            = R.PayloadView;

    DC->DataConnection->PostMessage(Cmd_DV_RL_PostData, 0, Push);
    return true;
}

bool xDeviceRelayService::OnProxyNotifyConnectionState(const ubyte * Payload, size_t PayloadSize) {
    auto N = xPR_ConnectionStateNotify();
    if (!N.Deserialize(Payload, PayloadSize)) {
        X_PERROR("invalid protocol");
        return false;
    }

    auto RCP = RelayConnectionManager.GetConnectionById(N.RelaySideConnectionId);
    if (!RCP) {
        X_DEBUG_PRINTF("relay connection not found");
        return true;
    }

    auto DC = DeviceManager.GetDeviceById(RCP->DeviceId);
    if (!DC) {
        X_DEBUG_PRINTF("Device connection not found");
        return true;
    }

    X_DEBUG_PRINTF("TotalUpload: %" PRIu64 ", TotalDumped: %" PRIu64 "", N.TotalUploadedBytes, N.TotalDumpedBytes);
    auto TN                   = xTR_ConnectionStateNotify();
    TN.DeviceSideConnectionId = RCP->DeviceSideConnectionId;
    TN.RelaySideConnectionId  = RCP->RelaySideConnectionId;
    TN.NewState               = xTR_ConnectionStateNotify::STATE_UPDATE_TRANSFER;
    TN.TotalReadBytes         = N.TotalDumpedBytes;
    TN.TotalWrittenBytes      = N.TotalUploadedBytes;
    DC->DataConnection->PostMessage(Cmd_DV_RL_ProxyClientNotify, 0, TN);

    return true;
}
