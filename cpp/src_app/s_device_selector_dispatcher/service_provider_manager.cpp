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

uint64_t xDSD_ServiceProviderManager::AddServer(uint64_t ConnectionId, const xDeviceSelectorServerInfo & SI) {
    DEBUG_LOG("");
    if (SI.DevicePoolId >= xDeviceSelectorServerInfo::MAX_DEVICE_POOL_ID) {
        return 0;
    }

    auto Id = ServerProviderAllocationPool.Acquire();
    if (!Id) {
        return 0;
    }
    auto & SP            = ServerProviderAllocationPool[Id];
    SP.ConnectionId      = ConnectionId;
    SP.ServiceProviderId = Id;
    SP.ServerInfo        = SI;
    Logger->I("NewServerProvider, ServerId=%" PRIx64 ", ServerConnectionId=%" PRIx64 "", SP.ServiceProviderId, SP.ConnectionId);

    if (!(SI.StrategyFlags & DSS_IPV4) && !(SI.StrategyFlags & DSS_IPV6)) {
        Logger->E("invalid service provider");
        return Id;
    }

    auto & Pool = Pools[SI.DevicePoolId];
    if (SI.StrategyFlags & DSS_DEVICE_PERSISTENT) {
        DEBUG_LOG("not supported service provider");
        return Id;
    } else {
        if (SI.StrategyFlags & DSS_IPV4) {
            auto & List = Pool.GenericV4List;
            List.AddTail(SP);
        }
        if (SI.StrategyFlags & DSS_IPV6) {
            auto & List = Pool.GenericV6List;
            List.AddTail(SP);
        }
    }

    return Id;
}

void xDSD_ServiceProviderManager::RemoveServer(uint64_t ServiceProviderId) {
    DEBUG_LOG("");
    ServerProviderAllocationPool.CheckAndRelease(ServiceProviderId);
}

auto xDSD_ServiceProviderManager::SelectServiceProvider(uint16_t PoolId, uint16_t StrategyFlags) -> const xDSD_ServerProvider * {
    assert(PoolId < xDeviceSelectorServerInfo::MAX_DEVICE_POOL_ID);
    auto & Pool = Pools[PoolId];

    if (StrategyFlags & DSS_DEVICE_PERSISTENT) {
        DEBUG_LOG("not supported");
        return nullptr;
    }

    if (StrategyFlags & DSS_IPV4) {
        auto & List = Pool.GenericV4List;
        auto   Ret  = static_cast<xDSD_ServerProvider *>(List.PopHead());
        if (!Ret) {
            return nullptr;
        }
        List.AddTail(*Ret);
        return Ret;
    }

    if (StrategyFlags & DSS_IPV6) {
        auto & List = Pool.GenericV6List;
        auto   Ret  = static_cast<xDSD_ServerProvider *>(List.PopHead());
        if (!Ret) {
            return nullptr;
        }
        List.AddTail(*Ret);
        return Ret;
    }

    return nullptr;
}
