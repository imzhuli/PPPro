#pragma once
#include "../lib_utils/all.hpp"

struct xDSD_StrategyListNode : xListNode {};
using xDSD_StrategyList = xList<xDSD_StrategyListNode>;

struct xDSD_ServerProviderPool {
    xDSD_StrategyList StrategyList[xDeviceSelectorServerInfo::MAX_STRATEGY_ID];
};

struct xDSD_ServerProvider : xDSD_StrategyListNode {
    uint64_t                  ConnectionId;
    uint64_t                  ServiceProviderId;
    xDeviceSelectorServerInfo ServerInfo;
};

class xDSD_ServiceProviderManager {
public:
    bool Init();
    void Clean();

    bool AddServer(uint64_t ConnectionId, const xDeviceSelectorServerInfo & SI);
    void RemoveServer(uint64_t ServiceProviderId);
    auto SelectServiceProvider(uint16_t PoolId, uint16_t StrategyId) -> const xDSD_ServerProvider *;

private:
    xDSD_ServerProviderPool                   Pools[xDeviceSelectorServerInfo::MAX_POOL_ID];
    xel::xIndexedStorage<xDSD_ServerProvider> ServerProviderAllocationPool;
};
