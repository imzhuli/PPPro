#pragma once
#include <pp_common/_.hpp>

class xCacheNodeBase;
class xCacheManager;

struct xCacheRequestContext {
    uint64_t  RequestId = 0;
    xVariable Value     = {};
    xVariable ValueEx   = {};
};
class xCacheRequestContextNode : public xListNode {
    friend class xCacheManager;
    xCacheRequestContext Context;
};
using xCacheUpdateRequestList = xList<xCacheRequestContextNode>;

class xCacheNode final : xListNode {
    friend class xList<xCacheNode>;
    friend class xCacheManager;

    enum eState : uint16_t {
        CACHE_STATE_INIT   = 0x00,
        CACHE_STATE_QUERY  = 0x01,
        CACHE_STATE_RESULT = 0x02,
    };

    uint64_t                CacheNodeId;
    uint64_t                TimestampMS;
    eState                  State = CACHE_STATE_INIT;
    std::string             Key;
    xCacheUpdateRequestList PendingRequestList;

    const void * DataPtr = nullptr;
};
using xCacheTimeoutList = xList<xCacheNode>;

class xCacheManager {

public:
    struct xLocalAudit {
        size_t CacheNew            = 0;
        size_t CacheLimit          = 0;
        size_t CacheDelete         = 0;
        size_t CacheHit            = 0;
        size_t CacheQuery          = 0;
        size_t CacheQueryLimit     = 0;
        size_t CacheQueryFailed    = 0;
        size_t CacheQueryCombined  = 0;
        size_t CacheQueryTimeout   = 0;
        size_t CacheQueryCancelled = 0;

        size_t DanglingResult = 0;
        size_t ValidResult    = 0;
        size_t BadResult      = 0;

        std::string ToString() const;
    };

    using xAsyncQueryProcedure       = std::function<bool(uint64_t CacheNodeId, const std::string & Key)>;
    using xOnImmediateResultCallback = std::function<void(const xCacheRequestContext & Context, const void * Data)>;
    using xOnAsyncResultCallback     = std::function<void(const xCacheRequestContext & Context, const void * Data)>;
    using xOnReleaseDataCallback     = std::function<void(uint64_t CacheNodeId, const void * Data)>;

    bool Init(size_t MaxCacheNodeCount = 1024, size_t RequestPoolSize = 1024);
    void Clean();
    void Tick(uint64_t NowMS);

    xAsyncQueryProcedure       AsyncQueryProcedure       = Noop<false>;
    xOnImmediateResultCallback OnImmediateResultCallback = Noop<>;
    xOnAsyncResultCallback     OnAsyncResultCallback     = Noop<>;
    xOnReleaseDataCallback     OnReleaseDataCallback     = Noop<>;

public:
    void PostAcquireCacheNodeRequest(const std::string & Key, const xCacheRequestContext & Context);
    auto GetLocalAudit() { return LocalAudit; }
    auto GetAndResetLocalAudit() { return Steal(LocalAudit); }
    void SetAsyncResultData(uint64_t CacheNodeId, const void * Data);

protected:
    void RemoveTimeoutCacheNodes();
    bool MakeAsyncQuery(uint64_t CacheNodeId, const std::string & Key) { return AsyncQueryProcedure(CacheNodeId, Key); }
    void OnImmediateResult(const xCacheRequestContext & Context, const void * Data) { OnImmediateResultCallback(Context, Data); }
    void OnAsyncResultData(const xCacheRequestContext & Context, const void * Data) { OnAsyncResultCallback(Context, Data); }
    void OnReleaseData(uint64_t CacheNodeId, const void * Data) { OnReleaseDataCallback(CacheNodeId, Data); }

private:
    void ReleaseCacheNode(xCacheNode * NP);

private:
    xTicker                                       Ticker;
    xIndexedStorage<xCacheNode>                   CacheNodePool;
    xMemoryPool<xCacheRequestContextNode>         CacheRequestPool;
    std::unordered_map<std::string, xCacheNode *> CacheMap;
    xCacheTimeoutList                             CacheTimeoutList;
    xCacheTimeoutList                             CacheQuickReleaseTimeoutList;
    xLocalAudit                                   LocalAudit;

    uint64_t CacheTimeoutMS             = 5 * 60'000;
    uint64_t CacheQuickReleaseTimeoutMS = 30'000;
};
