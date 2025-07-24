#pragma once
#include "../base.hpp"

struct xPPC_RequestSource {
    uint64_t SourceConnectionId;
    uint64_t SourceRequestId;
};

class xPPC_RequestIdPool {

public:
    bool Init(size_t PoolSize, uint64_t TimeoutMS = 1'000) {
        if (!RequestPairPool.Init(PoolSize)) {
            return false;
        }
        this->TimeoutMS = TimeoutMS;
        return true;
    }

    void Clean() {
        RequestPairPool.Clean();
    }

    void Tick() {
        Ticker.Update();
        ReleaseTimeoutNodes();
    }

    void Tick(uint64_t NowMS) {
        Ticker.Update(NowMS);
        ReleaseTimeoutNodes();
    }

    uint64_t Acquire(const xPPC_RequestSource & Source) {
        auto RPId = RequestPairPool.Acquire();
        if (RPId) {
            auto & Ref = RequestPairPool[RPId];

            Ref.RequestPairId = RPId;
            Ref.Source        = Source;

            Ref.RequestStartTimestampMS = Ticker();
            RequestPairTimeoutQueue.AddTail(Ref);
        }
        return RPId;
    }

    uint64_t AcquireWithTickerUpdate(const xPPC_RequestSource & Source) {
        Ticker.Update();
        return Acquire(Source);
    }

    bool CheckAndRelease(uint64_t RequestPairId, xPPC_RequestSource & Source) {
        auto PRP = RequestPairPool.CheckAndGet(RequestPairId);
        if (!PRP) {
            return false;
        }
        Source = PRP->Source;
        RequestPairPool.Release(RequestPairId);
        return true;
    }

private:
    void ReleaseTimeoutNodes() {
        while (auto P = static_cast<xRequestPair *>(RequestPairTimeoutQueue.PopHead([KP = Ticker() - TimeoutMS](const xRequestPairTimeoutNode & N) {
                   return N.RequestStartTimestampMS < KP;
               }))) {
            RequestPairPool.Release(P->RequestPairId);
        }
    };

private:
    struct xRequestPairTimeoutNode : xListNode {
        uint64_t RequestStartTimestampMS;
    };
    struct xRequestPair : xRequestPairTimeoutNode {
        uint64_t           RequestPairId;
        xPPC_RequestSource Source;
    };

    xTicker                        Ticker;
    uint64_t                       TimeoutMS;
    xIndexedStorage<xRequestPair>  RequestPairPool;
    xList<xRequestPairTimeoutNode> RequestPairTimeoutQueue;
};
