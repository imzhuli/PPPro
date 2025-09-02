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

    bool Prefer4 = ServerListDownloadAddress.Is4();
    bool Prefer6 = ServerListDownloadAddress.Is6();
    RuntimeAssert(Prefer4 || Prefer6);

    DispatcherServerInfoDownloader.OnUpdateServerInfoCallback = [this, Prefer4, Prefer6](const xRelayInfoDispatcherServerInfo & Info) {
        if (Prefer4) {
            RelayInfoDispatcherClient.UpdateTarget(Info.ObserverAddress4);
        } else if (Prefer6) {
            RelayInfoDispatcherClient.UpdateTarget(Info.ObserverAddress6);
        }
    };
    RelayInfoDispatcherClient.OnConnectedCallback =
        [this]() { /* RelayInfoDispatcherClient.PostMessage(Cmd_RegisterRelayInfoObserver, 0, XR(xPP_RegisterRelayInfoObserver())); */ };
    RelayInfoDispatcherClient.OnPacketCallback = [this](xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        return OnRelayInfoDispatcherPacket(CommandId, RequestId, PayloadPtr, PayloadSize);
    };
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
    while (auto RSI = RelayInfoTimeoutList.PopHead([KilliTimepoint](const xManagedRelayServerInfo & N) { return N.LastKeepAliveTimestampMS < KilliTimepoint; })) {
        RemoveDeviceRelayInfo(RSI);
    }
}

bool xRelayInfoObserver::OnRelayInfoDispatcherPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_BroadcastRelayInfo:
            return OnBroadcastRelayInfo(PayloadPtr, PayloadSize);
        case Cmd_BroadcastRelayOffline:
            return OnBroadcastRelayOffline(PayloadPtr, PayloadSize);
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

bool xRelayInfoObserver::OnBroadcastRelayOffline(ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_BroadcastRelayOffline();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    auto MapIter = DeviceRelayServerIdLocalMap.find(R.ServerId);
    if (MapIter == DeviceRelayServerIdLocalMap.end()) {  // not found:
        return true;
    }
    auto & RS = DeviceRelayInfoPool[MapIter->second];
    if (R.ServerStartupTimestampMS != RS.Context.ServerInfo.StartupTimestampMS) {
        // relay info conflict, do nothing;
        return true;
    }
    RemoveDeviceRelayInfo(&RS);
    return true;
}

void xRelayInfoObserver::InsertOrKeepAliveDeviceRelayInfo(const xRelayServerInfoBase & Info) {
    assert(Info.ServerType == eRelayServerType::DEVICE);
    auto MapIter = DeviceRelayServerIdLocalMap.find(Info.ServerId);
    if (MapIter != DeviceRelayServerIdLocalMap.end()) {  // keep alive
        auto & RS = DeviceRelayInfoPool[MapIter->second];
        if (Info.StartupTimestampMS == RS.Context.ServerInfo.StartupTimestampMS) {
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
        return;
    }
    auto & RS                   = DeviceRelayInfoPool[LocalId];
    RS.Context.ServerInfo       = Info;
    RS.LastKeepAliveTimestampMS = Ticker();
    RelayInfoTimeoutList.AddTail(RS);
    DeviceRelayServerIdLocalMap.insert(std::make_pair(Info.ServerId, LocalId));

    OnNewDeviceRelayInfoCallback(RS.Context);
}

void xRelayInfoObserver::RemoveDeviceRelayInfo(xManagedRelayServerInfo * RSI) {
    assert(RSI);
    assert(RSI->Context.ServerInfo.ServerId);
    assert(RSI->Context.ServerInfo.ServerType == eRelayServerType::DEVICE);

    OnRemoveDeviceRelayInfoCallback(RSI->Context);

    auto ServerId = RSI->Context.ServerInfo.ServerId;
    auto MapIter  = DeviceRelayServerIdLocalMap.find(ServerId);
    auto LocalId  = MapIter->second;
    assert(RSI == DeviceRelayInfoPool.CheckAndGet(LocalId));

    DeviceRelayServerIdLocalMap.erase(MapIter);
    DeviceRelayInfoPool.Release(LocalId);
}
