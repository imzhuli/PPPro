#pragma once
#include <pp_common/_.hpp>

struct xCC_RelayInfoReference {
    const xRelayServerInfoBase * Info;

    bool operator==(const xCC_RelayInfoReference & O) const { return Info->ServerId == O.Info->ServerId; }
    auto operator<=>(const xCC_RelayInfoReference & O) { return Info->ServerId <=> O.Info->ServerId; };
};

struct xCC_LocalAudit {
    uint64_t TotalDeviceRelayServerCount       = 0;
    uint64_t TotalNewDeviceRelayServerCount    = 0;
    uint64_t TotalRemoveDeviceRelayServerCount = 0;
    uint32_t TotalDeviceRelayTags              = 0;

    std::string ToString();

    uint64_t LastOutputTimestampMS = 0;
    void     Tick(uint64_t NowMS);
};