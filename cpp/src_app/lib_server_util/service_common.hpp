#pragma once
#include "./runtime_env.hpp"

#include <pp_common/_.hpp>

extern xRuntimeEnv  RuntimeEnv;
extern xLogger *    Logger;
extern xLogger *    AuditLogger;
extern xIoContext * ServiceIoContext;
extern uint64_t     ServiceIoLoopOnceTimeoutMS;
extern xTicker      ServiceTicker;
extern xRunState    ServiceRunState;

struct xRuntimeEnvGuard final : xNonCopyable {
    xRuntimeEnvGuard(int argc, char ** argv);
    ~xRuntimeEnvGuard();

    xRuntimeEnv * operator->() const;
};

template <typename... tTickerObserver>
void ServiceUpdateOnce(tTickerObserver &&... Observers) {
    ServiceTicker.Update();
    ServiceIoContext->LoopOnce(ServiceIoLoopOnceTimeoutMS);
    TickAll(ServiceTicker(), std::forward<tTickerObserver>(Observers)...);
}

/////////////////////////
struct xServiceRequestContext;
class xServiceRequestContextPool;

class xServiceRequestContext : private xListNode {
    friend class xList<xServiceRequestContext>;

public:
    uint64_t          RequestId;
    uint64_t          RequestTimestampMS;
    mutable xVariable RequestContext;
    mutable xVariable RequestContextEx;
};
using xServiceRequestContextList = xList<xServiceRequestContext>;

class xServiceRequestContextPool {
public:
    bool Init(size_t PoolSize);
    void Clean();
    void Tick(uint64_t NowMS) { RemoveTimeoutRequests(DefaultTimeoutMS, NowMS); }

    auto Acquire(xVariable RequestContext = {}, xVariable RequestContextEx = {}) -> const xServiceRequestContext *;
    auto CheckAndGet(uint64_t RequestId) -> const xServiceRequestContext *;
    void Release(const xServiceRequestContext * RCP);

    using xOnTimeoutRequestCallback = std::function<void(const xServiceRequestContext *)>;
    void SetRequestTimeoutMS(uint64_t TimeoutMS) { DefaultTimeoutMS = TimeoutMS > 500 ? TimeoutMS : 500; }
    void SetOnTimeoutRequestCallback(const xOnTimeoutRequestCallback & CB) { OnTimeoutRequestCallback = CB; }
    void ClearTimeoutRequestCallback() { OnTimeoutRequestCallback = &IgnoreTimeoutRequest; }

    void RemoveTimeoutRequests(uint64_t TimeoutMS, uint64_t NowMS = ServiceTicker()) {
        auto KillTimepoint = NowMS - TimeoutMS;
        while (auto P = (const xServiceRequestContext *)TimeoutList.PopHead([KillTimepoint](const xServiceRequestContext & N) { return N.RequestTimestampMS <= KillTimepoint; })) {
            OnTimeoutRequestCallback(P);
            Pool.Release(P->RequestId);
        }
    }

private:
    static void IgnoreTimeoutRequest(const xServiceRequestContext *) {}

private:
    xIndexedStorage<xServiceRequestContext> Pool;
    xServiceRequestContextList              TimeoutList;
    uint64_t                                DefaultTimeoutMS         = 2'000;
    xOnTimeoutRequestCallback               OnTimeoutRequestCallback = &IgnoreTimeoutRequest;
};

#ifndef NDEBUG
#define DEBUG_LOG(fmt, ...) Logger->D("%s:%i:%s\n" fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif
