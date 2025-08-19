#include "./_global.hpp"

#include "../lib_utils/all.hpp"

xNetAddress BindAddressForDevice4;
xNetAddress BindAddressForDevice6;
xNetAddress ServerListDownloadAddress;
std::string GeoInfoMapFilename;
std::string IpLocationDbFilename;

xCC_IpLocationManager IpLocationManager;

xCC_RelayV4List RelayV4List;
xCC_RelayV6List RelayV6List;

xUdpService ChallengeService4;
xUdpService ChallengeService6;

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

const xRelayServerInfoBase * GetRandomDeviceRelayServer4() {
    auto C = static_cast<xCC_RelayScheduleNode *>(RelayV4List.PopHead());
    if (!C) {
        return nullptr;
    }
    RelayV4List.AddTail(*C);
    return C->ServerInfo;
}

const xRelayServerInfoBase * GetRandomDeviceRelayServer6() {
    auto C = static_cast<xCC_RelayScheduleNode *>(RelayV6List.PopHead());
    if (!C) {
        return nullptr;
    }
    RelayV4List.AddTail(*C);
    return C->ServerInfo;
}
