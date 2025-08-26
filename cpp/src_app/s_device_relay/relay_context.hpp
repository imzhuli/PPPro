#pragma once
#include <pp_common/_.hpp>

struct xRL_RelayContextTimeoutNode : xListNode {
    uint64_t TimestampMS;
};

struct xRL_RelayContext : xRL_RelayContextTimeoutNode {
    uint64_t RelaySideContextId;
    uint64_t DeviceId;
    uint32_t DeviceSideContextId;
    uint64_t ProxyConnectionId;
    uint64_t ProxySideContextId;
};
using xRL_RelayContextTimeoutList = xList<xRL_RelayContext>;

extern void InitRelayContextPool(size_t MaxContextCount);
extern void CleanRelayContextPool();

extern xRL_RelayContext * AllocRelayContext();
extern void               ReleaseRelayContext(xRL_RelayContext *);
extern size_t             GetRelayContextCount();
extern void               KeepAlive(xRL_RelayContext *);
extern void               ReleaseTimeoutRelayContext();
