#pragma once
#include <pp_common/_.hpp>

struct xRD_RelayConnectionContext : xListNode {
    union {
        uint16_t TargetPort;
        uint64_t DeviceSideConnectionId;
    };
    uint64_t RelaySideConnectionId;
    uint64_t ProxySideConnectionId;

    uint64_t DeviceId;
    uint64_t ProxyConnectionId;  // PA
};

class xRD_RelayConnectionManager {
public:
    bool Init(size_t MaxConnectionSize);
    void Clean();
    void Tick(uint64_t NowMS);

    auto Create() -> xRD_RelayConnectionContext *;
    auto GetConnectionById(uint64_t RelaySideConnectionId) -> xRD_RelayConnectionContext *;
    void Destroy(xRD_RelayConnectionContext * RCC);

private:
    xTicker                                     Ticker;
    xIndexedStorage<xRD_RelayConnectionContext> ContextPool;
};
