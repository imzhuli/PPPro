#include "../lib_server_list/relay_info_observer.hpp"
#include "../lib_utils/all.hpp"
#include "./_global.hpp"

auto RelayInfoObserver = xRelayInfoObserver();

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = REG->LoadConfig();

    CL.Require(BindAddressForDevice, "BindAddressForDevice");
    CL.Optional(BindAddressForDeviceV6Test, "BindAddressForDeviceV6Test");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");
    CL.Require(GeoInfoMapFilename, "GeoInfoMapFilename");
    CL.Require(IpLocationDbFilename, "IpLocationDbFilename");

    X_GUARD(IpLocationManager, GeoInfoMapFilename, IpLocationDbFilename);
    X_GUARD(RelayInfoObserver, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(ChallengeChannel, ServiceIoContext, BindAddressForDevice, &ChallengeChennelReactor);
    X_COND_GUARD(BindAddressForDeviceV6Test, Ipv6TestChennel, ServiceIoContext, BindAddressForDeviceV6Test, &Ipv6TestChennelReactor);

    RelayInfoObserver.SetOnNewDeviceRelayInfoCallback([](const xRelayServerInfoBase * Info) {
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
    });

    RelayInfoObserver.SetOnRemoveDeviceRelayInfoCallback([](const xRelayServerInfoBase * Info) {
        auto TempRef  = xCC_RelayInfoReference{ Info };
        auto ListIter = TaggedDeviceRelayServerListMap.find(Info->ForcedPoolId);
        assert(ListIter != TaggedDeviceRelayServerListMap.end());

        auto & List = ListIter->second;
        auto   Iter = std::lower_bound(List.begin(), List.end(), TempRef);

        assert(Iter != List.end() && Iter->Info == Info);
        DEBUG_LOG("RemoveDeviceRelayInfo: %s", Iter->Info->ToString().c_str());

        List.erase(Iter);
        if (List.empty()) {
            --LocalAudit.TotalDeviceRelayTags;
            TaggedDeviceRelayServerListMap.erase(ListIter);
        }

        --LocalAudit.TotalDeviceRelayServerCount;
        ++LocalAudit.TotalRemoveDeviceRelayServerCount;
    });

    while (ServiceRunState) {
        ServiceUpdateOnce(RelayInfoObserver, LocalAudit);
    }

    return 0;
}
