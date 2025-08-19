#pragma once
#include "../lib_utils/all.hpp"
#include "./_local_audit.hpp"
#include "./challenge_service.hpp"
#include "./ip_location.hpp"
#include "./relay_node.hpp"

#include <map>
#include <pp_common/_.hpp>

extern xNetAddress BindAddressForDevice4;
extern xNetAddress BindAddressForDevice6;
extern xNetAddress ServerListDownloadAddress;
extern std::string GeoInfoMapFilename;
extern std::string IpLocationDbFilename;

extern xCC_IpLocationManager IpLocationManager;

extern xCC_RelayV4List RelayV4List;
extern xCC_RelayV6List RelayV6List;

extern xUdpService ChallengeService4;
extern xUdpService ChallengeService6;

extern xCC_LocalAudit LocalAudit;

using xCC_DeviceRelayList             = std::vector<xCC_RelayInfoReference>;
using xTaggedDeviceRelayServerListMap = std::map<uint32_t, xCC_DeviceRelayList>;
extern xTaggedDeviceRelayServerListMap TaggedDeviceRelayServerListMap;

extern uint32_t GeoInfoToForcedPoolId(const xGeoInfo & GeoInfo);

extern const xRelayServerInfoBase * GetRandomDeviceRelayServer4();
extern const xRelayServerInfoBase * GetRandomDeviceRelayServer6();
