#include "./relay_context.hpp"

#include "./_global.hpp"

#include <atomic>

static xel::xIndexedStorage<xRL_RelayContext> RelayContextPool;
static xRL_RelayContextTimeoutList            RelayContextIdleList;

void InitRelayContextPool(size_t MaxContextCount) {
    RuntimeAssert(RelayContextPool.Init(MaxContextCount));
}

void CleanRelayContextPool() {
    RelayContextPool.Clean();
}

xRL_RelayContext * AllocRelayContext() {
    auto Id = RelayContextPool.AcquireValue();
    if (!Id) {
        return nullptr;
    }
    auto & RC             = RelayContextPool[Id];
    RC.RelaySideContextId = Id;
    RC.TimestampMS        = ServiceTicker();
    RelayContextIdleList.AddTail(RC);
    return &RC;
}

void ReleaseRelayContext(xRL_RelayContext * PRC) {
    assert(RelayContextPool.CheckAndGet(PRC->RelaySideContextId) == PRC);
    RelayContextPool.Release(PRC->RelaySideContextId);
}

void KeepAlive(xRL_RelayContext * PRC) {
    PRC->TimestampMS = ServiceTicker();
    RelayContextIdleList.GrabTail(*PRC);
}

void ReleaseTimeoutRelayContext() {
    auto KillTimepoint = ServiceTicker() - RelayContextIdleTimeoutMS;
    while (auto PRC = static_cast<xRL_RelayContext *>(RelayContextIdleList.PopHead([KillTimepoint](auto & C) { return C.TimestampMS <= KillTimepoint; }))) {
        ReleaseRelayContext(PRC);
    }
}
