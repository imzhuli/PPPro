#include "./cache_manager.hpp"

#include "./service_common.hpp"

std::string xCacheManager::xLocalAudit::ToString() const {
    auto OS = std::ostringstream();
    OS << "CacheNew: " << CacheNew << endl;
    OS << "CacheLimit: " << CacheLimit << endl;
    OS << "CacheDelete: " << CacheDelete << endl;
    OS << "CacheHit: " << CacheHit << endl;
    OS << "CacheQuery: " << CacheQuery << endl;
    OS << "CacheQueryLimit: " << CacheQueryLimit << endl;
    OS << "CacheQueryFailed: " << CacheQueryFailed << endl;
    OS << "CacheQueryCombined: " << CacheQueryCombined << endl;
    OS << "CacheQueryTimeout: " << CacheQueryTimeout << endl;
    OS << "CacheQueryCancelled: " << CacheQueryCancelled << endl;
    OS << "DanglingResult: " << DanglingResult << endl;
    OS << "ValidResult: " << ValidResult << endl;
    OS << "BadResult: " << BadResult << endl;
    return OS.str();
}

bool xCacheManager::Init(size_t MaxCacheNodeSize, size_t RequestPoolSize) {
    if (!CacheNodePool.Init(MaxCacheNodeSize)) {
        return false;
    }
    if (!CacheRequestPool.Init(RequestPoolSize)) {
        CacheNodePool.Clean();
        return false;
    }
    return true;
}

void xCacheManager::Clean() {
    CacheRequestPool.Clean();
    CacheNodePool.Clean();
}

void xCacheManager::RemoveTimeoutCacheNodes(uint64_t NowMS) {
    auto QuickReleaseTimepoint = NowMS - CacheQuickReleaseTimeoutMS;
    while (auto P = CacheQuickReleaseTimeoutList.PopHead([QuickReleaseTimepoint](auto & C) { return C.TimestampMS < QuickReleaseTimepoint; })) {
        ReleaseCacheNode(P);
    }

    auto ReleaseTimepoint = NowMS - CacheTimeoutMS;
    while (auto P = CacheTimeoutList.PopHead([ReleaseTimepoint](auto & C) { return C.TimestampMS < ReleaseTimepoint; })) {
        ReleaseCacheNode(P);
    }
}

void xCacheManager::PostAcquireCacheNodeRequest(const std::string & Key, const xCacheRequestContext & Context) {
    auto NP   = (xCacheNode *)nullptr;
    auto Iter = CacheMap.find(Key);
    if (Iter == CacheMap.end()) {
        auto CNID = CacheNodePool.Acquire();
        if (!CNID) {
            ++LocalAudit.CacheLimit;
            return;
        }
        NP              = &CacheNodePool[CNID];
        NP->CacheNodeId = CNID;
        NP->Key         = Key;
        CacheMap[Key]   = NP;

        NP->TimestampMS = ServiceTicker();
        CacheQuickReleaseTimeoutList.AddTail(*NP);

        ++LocalAudit.CacheNew;
    } else {
        NP = Iter->second;
    }

    if (NP->State == xCacheNode::CACHE_STATE_RESULT) {
        OnImmediateResult(Context, NP->DataPtr);
        ++LocalAudit.CacheHit;
        return;
    }

    auto RCtx = CacheRequestPool.Create();
    if (!RCtx) {
        ++LocalAudit.CacheQueryLimit;
        OnImmediateResult(Context, nullptr);
        return;
    }
    RCtx->Context = Context;
    NP->PendingRequestList.AddTail(*RCtx);

    if (NP->State == xCacheNode::CACHE_STATE_QUERY) {
        ++LocalAudit.CacheQueryCombined;
        return;
    }

    assert(NP->State == xCacheNode::CACHE_STATE_INIT);
    if (!MakeAsyncQuery(NP->CacheNodeId, Key)) {
        ++LocalAudit.CacheQueryFailed;
        NP->DataPtr = nullptr;
        NP->State   = xCacheNode::CACHE_STATE_RESULT;
        OnImmediateResult(Context, nullptr);
        return;
    }
    ++LocalAudit.CacheQuery;
}

void xCacheManager::SetAsyncResultData(uint64_t CacheNodeId, const void * Data) {
    auto NP = CacheNodePool.CheckAndGet(CacheNodeId);
    if (!NP) {
        ++LocalAudit.DanglingResult;
        return;
    }
    if (auto ODP = Steal(NP->DataPtr)) {
        OnReleaseData(NP->CacheNodeId, ODP);
    }

    NP->DataPtr = Data;
    NP->State   = xCacheNode::CACHE_STATE_RESULT;
    if (Data) {
        CacheTimeoutList.GrabTail(*NP);
        ++LocalAudit.ValidResult;
    } else {
        ++LocalAudit.BadResult;
    }
    while (auto RCP = NP->PendingRequestList.PopHead()) {
        OnAsyncResultData(RCP->Context, NP->DataPtr);
        CacheRequestPool.Destroy(RCP);
    }
}

void xCacheManager::ReleaseCacheNode(xCacheNode * NP) {
    assert(NP && NP == CacheNodePool.CheckAndGet(NP->CacheNodeId));
    while (auto RCP = NP->PendingRequestList.PopHead()) {
        ++LocalAudit.CacheQueryCancelled;
        OnAsyncResultData(RCP->Context, nullptr);
        CacheRequestPool.Destroy(RCP);
    }
    if (NP->DataPtr) {
        --LocalAudit.ValidResult;
        OnReleaseData(NP->CacheNodeId, NP->DataPtr);
    } else {
        --LocalAudit.BadResult;
    }
}
