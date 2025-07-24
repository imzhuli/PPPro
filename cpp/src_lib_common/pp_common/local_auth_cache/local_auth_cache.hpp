#pragma once
#include "../_.hpp"

#include <unordered_map>

struct xPPC_LocalAuthLRUNode : xListNode {};

struct xPPC_LocalAuthTimeoutNode : xListNode {
    uint64_t TimestampMS         = 0;
    uint16_t RemainMinuteCounter = 0;
};

struct xPPC_ResourceSource {
    uint16_t PoolIndex;
    uint16_t PoolWeight;
};

struct xPPC_LocalAuthInfo : xPPC_LocalAuthTimeoutNode {
    xAuditId            AuditId;
    xContinentId        ContinentId;
    xCountryId          CountryId;
    xStateId            StateId;
    xCityId             CityId;
    bool                EnableUdp;
    bool                EnableIpv6;
    xPPC_ResourceSource Sources;
    std::string         ThirdSourceURL;
};

class xPPC_LocalAuthCache {
public:
    bool Init(size_t MaxCacheSize);
    void Clean();
    void Tick(uint64_t NowMS);

private:
    const xPPC_LocalAuthInfo * Query(const std::string & AuthPass);
    void                       CleanTimeoutNodes();
    //

private:
    xTicker                                                     Ticker;
    xIndexedStorage<xPPC_LocalAuthInfo>                         AuthInfoPool;
    std::unordered_map<std::string, const xPPC_LocalAuthInfo *> AuthInfoMap;

    xList<xPPC_LocalAuthLRUNode>     LRUList;
    xList<xPPC_LocalAuthTimeoutNode> TimeoutListByMinute[60];
    int_fast32_t                     CurrentTimeoutListIndex = 0;

    //
public:
    static constexpr const size_t NodeSize = sizeof(xPPC_LocalAuthInfo);
};
