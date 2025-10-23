#include "./proxy_manager.hpp"

#include "./_global.hpp"

#include <pp_protocol/device_relay/connection.hpp>
#include <pp_protocol/device_relay/post_data.hpp>
#include <pp_protocol/device_relay/udp_channel.hpp>
#include <pp_protocol/proxy_relay/connection.hpp>

static xTcpService ProxyService4;

void InitProxyService() {
    RuntimeAssert(ProxyService4.Init(ServiceIoContext, ProxyAddress4, 20000));
    ProxyService4.OnClientPacket = OnProxyPacket;
}

void CleanProxyService() {
    ProxyService4.Clean();
}

void TickProxyService(uint64_t NowMS) {
    ProxyService4.Tick(NowMS);
}

////////////////////

static bool OnProxyCreateConnection(const xTcpServiceClientConnectionHandle & CC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Request = xPR_CreateConnection();
    if (!Request.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("invalid protocol");
        return false;
    }

    auto Ctx = (xRL_RelayContext *)nullptr;
    if (Request.TargetAddress) {
        DEBUG_LOG("ByAddress");
        Ctx = CreateTcpConnection(Request.RelayServerSideDeviceId, Request.TargetAddress);
    } else {
        DEBUG_LOG("ByHost");
        Ctx = CreateTcpConnection(Request.RelayServerSideDeviceId, Request.HostnameView, Request.HostnamePort);
    }

    if (!Ctx) {
        auto Resp               = xPP_ProxyConnectionState();
        Resp.ProxySideContextId = Request.ProxySideContextId;
        CC.PostMessage(Cmd_PA_RL_NotifyConnectionState, 0, Resp);
        return true;
    }
    Ctx->ProxyConnectionId  = CC.GetConnectionId();
    Ctx->ProxySideContextId = Request.ProxySideContextId;
    return true;
}

static bool OnProxyDestroyConnection(const xTcpServiceClientConnectionHandle & CC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Request = xPR_DestroyConnection();
    if (!Request.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("invalid protocol");
        return false;
    }

    auto PRC = GetRelayContextById(Request.RelaySideContextId);
    if (!PRC) {
        DEBUG_LOG("context mismatch");
        return true;
    }
    PostDestroyConnection(PRC);
    ReleaseRelayContext(PRC);
    return true;
}

static bool OnProxyPushData(const xTcpServiceClientConnectionHandle & CC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Request = xPR_PushTcpData();
    if (!Request.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("invalid protocol");
        return false;
    }
    PostConnectionData(Request.RelaySideContextId, Request.PayloadView.data(), Request.PayloadView.size());
    return true;
}

bool OnProxyPacket(const xTcpServiceClientConnectionHandle & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    DEBUG_LOG("CommandId=%" PRIx32 "", CommandId);
    switch (CommandId) {
        case Cmd_PA_RL_CreateConnection:
            return OnProxyCreateConnection(CC, PayloadPtr, PayloadSize);
        case Cmd_PA_RL_PostData:
            return OnProxyPushData(CC, PayloadPtr, PayloadSize);
        case Cmd_PA_RL_DestroyConnection:
            return OnProxyDestroyConnection(CC, PayloadPtr, PayloadSize);
        case Cmd_PA_RL_CreateUdpBinding:
        case Cmd_PA_RL_DestroyUdpBinding:
        case Cmd_DV_RL_KeepAliveUdpChannel:
        case Cmd_DV_RL_PostUdpChannelData:
            Todo("");
            break;

        default:
            break;
    }
    return true;
}

void PostDataToProxy(uint64_t ProxyConnectionId, const void * PL, size_t PS) {
    auto H = ProxyService4.GetConnectionHandle(ProxyConnectionId);
    if (!H.operator->()) {
        DEBUG_LOG("mismatched proxy connection");
        return;
    }
    H.PostData(PL, PS);
}

void PostMessageToProxy(uint64_t ProxyConnectionId, xPacketCommandId CmdId, xPacketRequestId ReqId, xel::xBinaryMessage & Message) {
    auto H = ProxyService4.GetConnectionHandle(ProxyConnectionId);
    if (!H.operator->()) {
        DEBUG_LOG("mismatched proxy connection");
        return;
    }
    H.PostMessage(CmdId, ReqId, Message);
}
