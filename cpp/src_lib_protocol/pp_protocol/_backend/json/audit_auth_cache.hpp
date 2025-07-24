#pragma once
#include "../../base.hpp"

struct xAudit_AuthCache {

    const char * AuditName = "xAudit_AuthCache";

    std::string ServerUuid;
    uint64_t    ServerLocalTimestampMS;
    uint64_t    AuditDuration;

    uint64_t AuthQueryCount;
    uint64_t CacheHitCount;
    uint64_t RemovedCacheNodeCount;  // timeout nodes

    uint64_t QueryCount;
    uint64_t MaxQueryTimeout;
    uint64_t AverageQueryTimeout;

    //
};
