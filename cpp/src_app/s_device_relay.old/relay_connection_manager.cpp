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
    auto & Ref             = ContextPool[Id];
    Ref.RelaySideContextId = Id;
    return &Ref;
}

auto xRD_RelayConnectionManager::GetConnectionById(uint64_t RelaySideContextId) -> xRD_RelayConnectionContext * {
    return ContextPool.CheckAndGet(RelaySideContextId);
}

void xRD_RelayConnectionManager::Destroy(xRD_RelayConnectionContext * RCC) {
    ContextPool.CheckAndRelease(RCC->RelaySideContextId);
}