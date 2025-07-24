#pragma once
#include "../lib_server_util/all.hpp"
#include "./challenge_service.hpp"
#include "./ip_location.hpp"

#include <map>
#include <pp_common/base.hpp>

extern xNetAddress BindAddressForDevice;
extern xNetAddress BindAddressForDeviceV6Test;
extern xNetAddress ServerListDownloadAddress;
extern std::string GeoInfoMapFilename;
extern std::string IpLocationDbFilename;

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
extern xCC_LocalAudit LocalAudit;

using xCC_DeviceRelayList             = std::vector<xCC_RelayInfoReference>;
using xTaggedDeviceRelayServerListMap = std::map<uint32_t, xCC_DeviceRelayList>;
extern xTaggedDeviceRelayServerListMap TaggedDeviceRelayServerListMap;

extern uint32_t                     GeoInfoToForcedPoolId(const xGeoInfo & GeoInfo);
extern const xRelayServerInfoBase * GetRandomRelayServerInfoByRegion(const xGeoInfo & GeoInfo);

extern xCC_IpLocationManager IpLocationManager;

extern xUdpChannel                 ChallengeChannel;
extern xCC_ChallengeChennelReactor ChallengeChennelReactor;
extern xUdpChannel                 Ipv6TestChennel;
extern xCC_Ipv6TestChennelReactor  Ipv6TestChennelReactor;