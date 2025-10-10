#include "./relay_connection_manager.hpp"

#include "../lib_server_list/relay_info_observer.hpp"
#include "./_global.hpp"

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
    RV = Cid;
    DEBUG_LOG("RelayServer added: LocalId=%" PRIx64 ", ServerId=%" PRIx64 ", Address=%s", Info.ServerInfo.ServerId, Info.ServerInfo.ExportProxyAddress4.ToString().c_str());
}

static void OnRemoveDeviceRelayInfoCallback(const xRIO_RelayServerInfoContext & Info) {
    auto Iter = RelayIdMap.find(Info.ServerInfo.ServerId);
    RuntimeAssert(Iter != RelayIdMap.end());
    RelayIdMap.erase(Iter);

    auto Cid = Info.MutableUserContext.U64;
    RelayPool.RemoveServer(Cid);
}

static bool OnRelayData(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    DEBUG_LOG("CommandId=%" PRIx32 ", RequestId=%" PRIx64 " ", CommandId, RequestId);

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

bool RequestRelayTargetConnection(uint64_t RelayServerId, uint64_t DeviceRelaySideId, const xNetAddress & TargetAddress) {
    return false;
}
