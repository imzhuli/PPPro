#include "./_global.hpp"

#include "../lib_utils/all.hpp"

xNetAddress BindAddressForDevice4;
xNetAddress BindAddressForDevice6;
xNetAddress ServerListDownloadAddress;
std::string GeoInfoMapFilename;
std::string IpLocationDbFilename;

xCC_LocalAudit LocalAudit;

std::string xCC_LocalAudit::ToString() {
    auto OS = std::ostringstream();
    OS << "TotalDRSCount:" << TotalDeviceRelayServerCount << " ";
    OS << "TotalNewDRSCount:" << TotalNewDeviceRelayServerCount << " ";
    OS << "TotalRemoveDRSCount:" << TotalRemoveDeviceRelayServerCount << " ";
    OS << "TotalRemoveDRSCount:" << TotalRemoveDeviceRelayServerCount << " ";
    OS << "TotalRemoveDRSTagCount:" << TotalDeviceRelayTags << " ";
    return OS.str();
}

void xCC_LocalAudit::Tick(uint64_t NowMS) {
    if (NowMS - LastOutputTimestampMS <= 60'000) {
        return;
    }
    AuditLogger->I("LocalAudit: %s", ToString().c_str());
    LastOutputTimestampMS = NowMS;
}

xTaggedDeviceRelayServerListMap TaggedDeviceRelayServerListMap;

uint32_t GeoInfoToForcedPoolId(const xGeoInfo & GeoInfo) {
    return 0;
}

const xRelayServerInfoBase * GetRandomRelayServerInfoByRegion(const xGeoInfo & GeoInfo) {
    auto RID  = GeoInfoToForcedPoolId(GeoInfo);
    auto Iter = TaggedDeviceRelayServerListMap.find(RID);
    if (Iter == TaggedDeviceRelayServerListMap.end()) {
        return nullptr;
    }
    auto & List = Iter->second;
    assert(List.size());

    auto Index = rand() % List.size();
    return List[Index].Info;
}

xCC_IpLocationManager IpLocationManager;

xel::xUdpService ChallengeService4;
xel::xUdpService ChallengeService6;