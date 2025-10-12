#include "./relay_connection_manager.hpp"

#include "../lib_server_list/relay_info_observer.hpp"
#include "./_global.hpp"

#include <pp_protocol/proxy_relay/connection.hpp>

static auto RIO       = xRelayInfoObserver();
static auto RelayPool = xClientPool();

// relay server id ---> client pool connection id
static auto RelayIdMap = std::map<uint64_t, uint64_t>();

static void OnNewDeviceRelayInfoCallback(const xRIO_RelayServerInfoContext & Info) {
    auto Cid = RelayPool.AddServer(Info.ServerInfo.ExportProxyAddress4);
    if (!Cid) {
        Logger->E("RelayObserved but not added: ServerId=%" PRIx64 ", Address=%s", Info.ServerInfo.ServerId, Info.ServerInfo.ExportProxyAddress4.ToString().c_str());
        return;
    }
    Info.MutableUserContext.U64 = Cid;
    auto & RV                   = RelayIdMap[Info.ServerInfo.ServerId];
    RuntimeAssert(!RV);

    DEBUG_LOG("RelayServer added: LocalId=%" PRIx64 ", ServerId=%" PRIx64 ", Address=%s", Cid, Info.ServerInfo.ServerId, Info.ServerInfo.ExportProxyAddress4.ToString().c_str());
    RV = Cid;
}

static void OnRemoveDeviceRelayInfoCallback(const xRIO_RelayServerInfoContext & Info) {
    auto Iter = RelayIdMap.find(Info.ServerInfo.ServerId);
    RuntimeAssert(Iter != RelayIdMap.end());
    RelayIdMap.erase(Iter);

    auto Cid = Info.MutableUserContext.U64;
    DEBUG_LOG("RelayServer removed: LocalId=%" PRIx64 ", ServerId=%" PRIx64 ", Address=%s", Cid, Info.ServerInfo.ServerId, Info.ServerInfo.ExportProxyAddress4.ToString().c_str());
    RelayPool.RemoveServer(Cid);
}

static bool OnConnectionStateChange(ubyte * PayloadPtr, size_t PayloadSize) {
    auto Notify = xPP_ProxyConnectionState();
    if (!Notify.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }

    auto CC = GetClientConnectionById(Notify.ProxySideContextId);
    if (!CC) {
        DEBUG_LOG("mismatched connection");
        return true;
    }

    switch (CC->State) {

        case CS_S5_WAIT_FOR_CONECTION_ESTABLISH:
            OnPAC_S5_ConnectionResult(CC, Notify.RelaySideContextId);
            break;

        case CS_T_WAIT_FOR_CONECTION_ESTABLISH:
            OnPAC_T_ConnectionResult(CC, Notify.RelaySideContextId);
            break;

        default:
            DEBUG_LOG("unprocessed");
            break;
    }

    return true;
}

static bool OnRelayPushConnectionData(ubyte * PayloadPtr, size_t PayloadSize) {
    auto Push = xPR_PushData();
    if (!Push.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }
    auto CC = GetClientConnectionById(Push.ProxySideContextId);
    if (!CC) {
        DEBUG_LOG("client connection not match");
        return true;
    }

    DEBUG_LOG("PushData\n%s", HexShow(Push.PayloadView).c_str());
    CC->PostData(Push.PayloadView.data(), Push.PayloadView.size());
    KeepAlive(CC);
    return true;
}

static bool OnRelayDestroyConnection(ubyte * PayloadPtr, size_t PayloadSize) {
    auto N = xPR_DestroyConnection();
    if (!N.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }
    auto CC = GetClientConnectionById(N.ProxySideContextId);
    if (!CC) {
        DEBUG_LOG("client connection not match");
        return true;
    }

    DEBUG_LOG("DestroyConnection %" PRIx64 "", N.ProxySideContextId);
    SchedulePassiveKillClientConnection(CC);
    return true;
}

static bool OnRelayData(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    DEBUG_LOG("CommandId=%" PRIx32 ", RequestId=%" PRIx64 " ", CommandId, RequestId);
    switch (CommandId) {
        case Cmd_PA_RL_NotifyConnectionState:
            return OnConnectionStateChange(PayloadPtr, PayloadSize);
        case Cmd_PA_RL_PostData:
            return OnRelayPushConnectionData(PayloadPtr, PayloadSize);
        case Cmd_PA_RL_DestroyConnection:
            return OnRelayDestroyConnection(PayloadPtr, PayloadSize);

        default:
            DEBUG_LOG("unprocessed");
            break;
    }

    return true;
}

void InitRelayConnectionManager() {
    RuntimeAssert(RelayPool.Init(ServiceIoContext, 4000));
    RelayPool.OnServerPacket = OnRelayData;

    RuntimeAssert(RIO.Init(ServiceIoContext, ConfigServerListDownloadAddress));
    RIO.OnNewDeviceRelayInfoCallback    = OnNewDeviceRelayInfoCallback;
    RIO.OnRemoveDeviceRelayInfoCallback = OnRemoveDeviceRelayInfoCallback;
}

void CleanRelayConnectionManager() {
    RelayIdMap.clear();
    RelayPool.Clean();
    RIO.Clean();
}

void TickRelayConnectionManager(uint64_t NowMS) {
    TickAll(NowMS, RIO, RelayPool);
}

bool PostRelayMessage(uint64_t RelayServerId, xPacketCommandId CmdId, xPacketRequestId RequestId, xel::xBinaryMessage & Message) {
    auto Iter = RelayIdMap.find(RelayServerId);
    if (Iter == RelayIdMap.end()) {
        return false;
    }
    auto ConnectionId = Iter->second;
    return RelayPool.PostMessage(ConnectionId, CmdId, RequestId, Message);
}

bool RequestRelayTargetConnection(uint64_t ProxySideContextId, uint64_t RelayServerId, uint64_t DeviceRelaySideId, const xNetAddress & TargetAddress) {
    auto Request = xPR_CreateConnection();

    Request.RelayServerSideDeviceId = DeviceRelaySideId;
    Request.ProxySideContextId      = ProxySideContextId;
    Request.TargetAddress           = TargetAddress;

    return PostRelayMessage(RelayServerId, Cmd_PA_RL_CreateConnection, 0, Request);
}

bool RequestRelayTargetConnection(uint64_t ProxySideContextId, uint64_t RelayServerId, uint64_t DeviceRelaySideId, const std::string_view & TargetHost, uint16_t TargetPort) {
    auto Request = xPR_CreateConnection();

    Request.RelayServerSideDeviceId = DeviceRelaySideId;
    Request.ProxySideContextId      = ProxySideContextId;
    Request.HostnameView            = TargetHost;
    Request.HostnamePort            = TargetPort;

    return PostRelayMessage(RelayServerId, Cmd_PA_RL_CreateConnection, 0, Request);
}

void RequestRelayCloseConnection(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t RelaySideContextId) {
    auto Request               = xPR_DestroyConnection();
    Request.ProxySideContextId = ProxyConnectionId;
    Request.RelaySideContextId = RelaySideContextId;
    PostRelayMessage(RelayServerId, Cmd_PA_RL_DestroyConnection, 0, Request);
}