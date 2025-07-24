#include "./relay_connection_manager.hpp"

bool xRD_RelayConnectionManager::Init(size_t MaxConnectionSize) {
    Ticker.Update();
    return ContextPool.Init(MaxConnectionSize);
}

void xRD_RelayConnectionManager::Clean() {
    ContextPool.Clean();
}

void xRD_RelayConnectionManager::Tick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    // TODO: timeout connections
}

auto xRD_RelayConnectionManager::Create() -> xRD_RelayConnectionContext * {
    auto Id = ContextPool.Acquire();
    if (!Id) {
        return nullptr;
    }
    auto & Ref                = ContextPool[Id];
    Ref.RelaySideConnectionId = Id;
    return &Ref;
}

auto xRD_RelayConnectionManager::GetConnectionById(uint64_t RelaySideConnectionId) -> xRD_RelayConnectionContext * {
    return ContextPool.CheckAndGet(RelaySideConnectionId);
}

void xRD_RelayConnectionManager::Destroy(xRD_RelayConnectionContext * RCC) {
    ContextPool.CheckAndRelease(RCC->RelaySideConnectionId);
}