#include "../lib_server_list/relay_info_observer.hpp"
#include "../lib_utils/all.hpp"
#include "./_global.hpp"

static auto RelayInfoObserver = xRelayInfoObserver();

static bool Enable4 = false;
static bool Enable6 = false;

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = REG->LoadConfig();

    CL.Optional(BindAddressForDevice4, "BindAddressForDevice4");
    CL.Optional(BindAddressForDevice6, "BindAddressForDevice6");

    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");
    CL.Require(GeoInfoMapFilename, "GeoInfoMapFilename");
    CL.Require(IpLocationDbFilename, "IpLocationDbFilename");

    Enable4 = BindAddressForDevice4.IsV4() && BindAddressForDevice4.Port;
    Enable6 = BindAddressForDevice6.IsV6() && BindAddressForDevice6.Port;
    if (!Enable4 && !Enable6) {
        Logger->F("neither ipv4 or ipv6 is enabled");
        return 0;
    }
    X_COND_GUARD(Enable4, ChallengeService4, ServiceIoContext, BindAddressForDevice4);
    X_COND_GUARD(Enable6, ChallengeService6, ServiceIoContext, BindAddressForDevice6);

    X_GUARD(IpLocationManager, GeoInfoMapFilename, IpLocationDbFilename);
    X_GUARD(RelayInfoObserver, ServiceIoContext, ServerListDownloadAddress);

    RelayInfoObserver.OnNewDeviceRelayInfoCallback = [](const xRelayServerInfoBase * Info) {
        auto TempRef = xCC_RelayInfoReference{ Info };

        auto & List       = TaggedDeviceRelayServerListMap[Info->ForcedPoolId];
        auto   InsertIter = std::lower_bound(List.begin(), List.end(), TempRef);
        RuntimeAssert(InsertIter == List.end() || *InsertIter != TempRef);
        auto NewIter = List.insert(InsertIter, TempRef);

        if (List.size() == 1) {
            ++LocalAudit.TotalDeviceRelayTags;
        }
        ++LocalAudit.TotalDeviceRelayServerCount;
        ++LocalAudit.TotalNewDeviceRelayServerCount;
        DEBUG_LOG("NewDeviceRelayInfo: %s", NewIter->Info->ToString().c_str());
    };

    // RelayInfoObserver.SetOnRemoveDeviceRelayInfoCallback([](const xRelayServerInfoBase * Info) {
    //     auto TempRef  = xCC_RelayInfoReference{ Info };
    //     auto ListIter = TaggedDeviceRelayServerListMap.find(Info->ForcedPoolId);
    //     assert(ListIter != TaggedDeviceRelayServerListMap.end());

    //     auto & List = ListIter->second;
    //     auto   Iter = std::lower_bound(List.begin(), List.end(), TempRef);

    //     assert(Iter != List.end() && Iter->Info == Info);
    //     DEBUG_LOG("RemoveDeviceRelayInfo: %s", Iter->Info->ToString().c_str());

    //     List.erase(Iter);
    //     if (List.empty()) {
    //         --LocalAudit.TotalDeviceRelayTags;
    //         TaggedDeviceRelayServerListMap.erase(ListIter);
    //     }

    //     --LocalAudit.TotalDeviceRelayServerCount;
    //     ++LocalAudit.TotalRemoveDeviceRelayServerCount;
    // });

    while (ServiceRunState) {
        ServiceUpdateOnce(RelayInfoObserver, LocalAudit);
    }

    return 0;
}
