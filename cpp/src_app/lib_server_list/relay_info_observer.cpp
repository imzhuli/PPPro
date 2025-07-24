#include "./relay_info_observer.hpp"

bool xRelayInfoObserver::Init(xIoContext * ICP, const xNetAddress & ServerListDownloadAddress) {
    assert(DeviceRelayServerIdLocalMap.empty());
    RuntimeAssert(DeviceRelayInfoPool.Init(MAX_DEVICE_RELAY_SERVER_SUPPORTED));
    if (!RelayInfoDispatcherClient.Init(ICP)) {
        return false;
    }
    if (!DispatcherServerInfoDownloader.Init(ICP, ServerListDownloadAddress)) {
        RelayInfoDispatcherClient.Clean();
        return false;
    }
    DispatcherServerInfoDownloader.SetUpdateRelayInfoDispatcherServerInfoCallback([this](const xRelayInfoDispatcherServerInfo & Info) {
        RelayInfoDispatcherClient.UpdateTarget(Info.ObserverAddress);
    });
    RelayInfoDispatcherClient.SetOnConnectedCallback([this]() { RelayInfoDispatcherClient.PostMessage(Cmd_RegisterRelayInfoObserver, 0, XR(xPP_RegisterRelayInfoObserver())); });
    RelayInfoDispatcherClient.SetOnPacketCallback([this](xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        return OnRelayInfoDispatcherPacket(CommandId, RequestId, PayloadPtr, PayloadSize);
    });
    return true;
}

void xRelayInfoObserver::Clean() {
    DispatcherServerInfoDownloader.Clean();
    RelayInfoDispatcherClient.Clean();
    Reset(DeviceRelayServerIdLocalMap);
    DeviceRelayInfoPool.Clean();
}

void xRelayInfoObserver::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    TickAll(Ticker(), RelayInfoDispatcherClient, DispatcherServerInfoDownloader);

    auto KilliTimepoint = NowMS - RELAY_INFO_TIMEOUT_MS;
    while (auto RSI = RelayInfoTimeoutList.PopHead([KilliTimepoint](const xRelayServerInfo & N) { return N.LastKeepAliveTimestampMS < KilliTimepoint; })) {
        RemoveDeviceRelayInfo(RSI);
    }
}

bool xRelayInfoObserver::OnRelayInfoDispatcherPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_BroadcastRelayInfo:
            return OnBroadcastRelayInfo(PayloadPtr, PayloadSize);

        default:
            break;
    }

    return true;
}

bool xRelayInfoObserver::OnBroadcastRelayInfo(ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_BroadcastRelayInfo();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    InsertOrKeepAliveDeviceRelayInfo(R.ServerInfo);
    return true;
}

void xRelayInfoObserver::InsertOrKeepAliveDeviceRelayInfo(const xRelayServerInfoBase & Info) {
    assert(Info.ServerType == eRelayServerType::DEVICE);
    auto MapIter = DeviceRelayServerIdLocalMap.find(Info.ServerId);
    if (MapIter != DeviceRelayServerIdLocalMap.end()) {  // keep alive
        auto & RS = DeviceRelayInfoPool[MapIter->second];
        if (Info.StartupTimestampMS == RS.Info.StartupTimestampMS) {
            RS.LastKeepAliveTimestampMS = Ticker();
            RelayInfoTimeoutList.GrabTail(RS);
            return;
        }
        // relay info conflict, remove the current one;
        RemoveDeviceRelayInfo(&RS);
    }
    // new:
    auto LocalId = DeviceRelayInfoPool.Acquire();
    if (!LocalId) {
        if (OnNewDeviceRelayInfoErrorCallback) {
            OnNewDeviceRelayInfoErrorCallback(&Info);
        }
        return;
    }
    auto & RS                   = DeviceRelayInfoPool[LocalId];
    RS.Info                     = Info;
    RS.LastKeepAliveTimestampMS = Ticker();
    RelayInfoTimeoutList.AddTail(RS);
    DeviceRelayServerIdLocalMap.insert(std::make_pair(Info.ServerId, LocalId));

    if (OnNewDeviceRelayInfoCallback) {
        OnNewDeviceRelayInfoCallback(&RS.Info);
    }
}

void xRelayInfoObserver::RemoveDeviceRelayInfo(xRelayServerInfo * RSI) {
    assert(RSI);
    assert(RSI->Info.ServerId);
    assert(RSI->Info.ServerType == eRelayServerType::DEVICE);

    if (OnRemoveDeviceRelayInfoCallback) {
        OnRemoveDeviceRelayInfoCallback(&RSI->Info);
    }

    auto ServerId = RSI->Info.ServerId;
    auto MapIter  = DeviceRelayServerIdLocalMap.find(ServerId);
    auto LocalId  = MapIter->second;
    assert(RSI == DeviceRelayInfoPool.CheckAndGet(LocalId));

    DeviceRelayServerIdLocalMap.erase(MapIter);
    DeviceRelayInfoPool.Release(LocalId);
}
