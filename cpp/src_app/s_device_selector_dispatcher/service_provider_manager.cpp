#include "./service_provider_manager.hpp"

bool xDSD_ServiceProviderManager::Init() {
    if (!ServerProviderAllocationPool.Init(MAX_DEVICE_SELECTOR_COUNT)) {
        return false;
    }
    return true;
}

void xDSD_ServiceProviderManager::Clean() {
    ServerProviderAllocationPool.Clean();
}

bool xDSD_ServiceProviderManager::AddServer(uint64_t ConnectionId, const xDeviceSelectorServerInfo & SI) {
    if (SI.PoolId >= xDeviceSelectorServerInfo::MAX_POOL_ID || SI.StrategyFlags >= xDeviceSelectorServerInfo::MAX_STRATEGY_COMBINED_ENABLED_COUNT) {
        return false;
    }

    auto Id = ServerProviderAllocationPool.Acquire();
    if (!Id) {
        return false;
    }
    auto & SP            = ServerProviderAllocationPool[Id];
    SP.ConnectionId      = ConnectionId;
    SP.ServiceProviderId = Id;
    SP.ServerInfo        = SI;

    auto &   Pool       = Pools[SI.PoolId];
    uint16_t StrategyId = SI.StrategyFlags;
    auto &   SL         = Pool.StrategyList[StrategyId];
    SL.AddTail(SP);
    return true;
}

void xDSD_ServiceProviderManager::RemoveServer(uint64_t ServiceProviderId) {
    ServerProviderAllocationPool.CheckAndRelease(ServiceProviderId);
}

auto xDSD_ServiceProviderManager::SelectServiceProvider(uint16_t PoolId, uint16_t StrategyId) -> const xDSD_ServerProvider * {
    assert(PoolId < Length(Pools));
    auto & Pool = Pools[PoolId];

    assert(StrategyId < Length(Pool.StrategyList));
    auto & SL = Pool.StrategyList[StrategyId];

    auto Ret = static_cast<xDSD_ServerProvider *>(SL.PopHead());
    if (!Ret) {
        return nullptr;
    }

    SL.AddTail(*Ret);
    return Ret;
}
