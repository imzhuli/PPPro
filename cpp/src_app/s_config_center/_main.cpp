#include "../lib_server_list/relay_info_observer.hpp"
#include "../lib_utils/all.hpp"
#include "./_global.hpp"
#include "./challenge_service.hpp"

static auto RelayInfoObserver = xRelayInfoObserver();

static bool Enable4 = false;
static bool Enable6 = false;

int main(int argc, char ** argv) {
    X_VAR xRuntimeEnvGuard(argc, argv);
    auto  CL = RuntimeEnv.LoadConfig();

    CL.Optional(BindAddressForDevice4, "BindAddressForDevice4");
    CL.Optional(BindAddressForDevice6, "BindAddressForDevice6");

    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");
    CL.Require(GeoInfoMapFilename, "GeoInfoMapFilename");
    CL.Require(IpLocationDbFilename, "IpLocationDbFilename");

    Enable4 = BindAddressForDevice4.Is4() && BindAddressForDevice4.Port;
    Enable6 = BindAddressForDevice6.Is6() && BindAddressForDevice6.Port;
    if (!Enable4 && !Enable6) {
        Logger->F("neither ipv4 or ipv6 is enabled");
        return 0;
    }
    Logger->I("bind v4: %s", BindAddressForDevice4.ToString().c_str());
    Logger->I("bind v6: %s", BindAddressForDevice6.ToString().c_str());

    ChallengeService4.OnPacketCallback = OnTerminalChallenge;
    ChallengeService6.OnPacketCallback = OnTerminalChallenge;

    X_COND_GUARD(Enable4, ChallengeService4, ServiceIoContext, BindAddressForDevice4);
    X_COND_GUARD(Enable6, ChallengeService6, ServiceIoContext, BindAddressForDevice6);

    X_GUARD(IpLocationManager, GeoInfoMapFilename, IpLocationDbFilename);
    X_GUARD(RelayInfoObserver, ServiceIoContext, ServerListDownloadAddress);

    RelayInfoObserver.OnNewDeviceRelayInfoCallback = [](const xRIO_RelayServerInfoContext & Context) {
        DEBUG_LOG("NewDeviceRelayInfo: %s", Context.ServerInfo.ToString().c_str());
        auto SN = new xCC_RelayScheduleNode();

        SN->ServerInfo = &Context.ServerInfo;
        if (SN->ServerInfo->ExportDeviceAddress4) {
            RelayV4List.AddTail(*SN);
        }
        if (SN->ServerInfo->ExportDeviceAddress6) {
            RelayV6List.AddTail(*SN);
        }
        Context.MutableUserContext.P = SN;
        ++LocalAudit.TotalDeviceRelayServerCount;
    };

    RelayInfoObserver.OnRemoveDeviceRelayInfoCallback = [](const xRIO_RelayServerInfoContext & Context) {
        auto SN = static_cast<xCC_RelayScheduleNode *>(Steal(Context.MutableUserContext.P));
        delete SN;
        --LocalAudit.TotalDeviceRelayServerCount;
    };

    while (ServiceRunState) {
        ServiceUpdateOnce(RelayInfoObserver, LocalAudit, IpLocationManager);
    }

    return 0;
}
