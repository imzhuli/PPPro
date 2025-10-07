#pragma once
#include "../lib_utils/all.hpp"

struct xDSD_GenericV4ListNode : xListNode {};
struct xDSD_GenericV6ListNode : xListNode {};
struct xDSD_NoDeviceChangeV4ListNode : xListNode {};
struct xDSD_NoDeviceChangeV6ListNode : xListNode {};

struct xDSD_ServerProviderPool {
    xList<xDSD_GenericV4ListNode>        GenericV4List;
    xList<xDSD_GenericV6ListNode>        GenericV6List;
    xList<xDSD_NoDeviceChangeV4ListNode> NoDeviceChangeV4List;
    xList<xDSD_NoDeviceChangeV6ListNode> NoDeviceChangeV6List;
};

struct xDSD_ServerProvider
    : xDSD_GenericV4ListNode
    , xDSD_GenericV6ListNode
    , xDSD_NoDeviceChangeV4ListNode
    , xDSD_NoDeviceChangeV6ListNode {

    uint64_t                  ConnectionId;
    uint64_t                  ServiceProviderId;
    xDeviceSelectorServerInfo ServerInfo;
};

class xDSD_ServiceProviderManager {
public:
    bool Init();
    void Clean();

    uint64_t AddServer(uint64_t ConnectionId, const xDeviceSelectorServerInfo & SI);
    void     RemoveServer(uint64_t ServiceProviderId);
    auto     SelectServiceProvider(uint16_t PoolId, uint16_t StrategyFlags) -> const xDSD_ServerProvider *;

private:
    xDSD_ServerProviderPool                   Pools[xDeviceSelectorServerInfo::MAX_DEVICE_POOL_ID];
    xel::xIndexedStorage<xDSD_ServerProvider> ServerProviderAllocationPool;
};
