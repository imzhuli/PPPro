#pragma once
#include "../lib_server_util/all.hpp"

#include <bitset>

struct xDSD_ServiceProviderInfo {
    enum eRegionDetailLevel {
        UNSPECIFIED = 0,
        COUNTRY     = 1,
        STATE       = 2,
        CITY        = 3,
        LOCAL_AREA  = 4,
    };
    eRegionDetailLevel RegionDetailLevel    = UNSPECIFIED;
    bool               AllowRegionDowngrade = false;  // 当高精度区域不可用时, 返回粗精度的IP
    bool               HasAuditBinding      = false;  // 对于要求IP不变的对象, 需要有保持记忆功能

    std::bitset<128> PoolFlags = {};
    //
};
