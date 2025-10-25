#include "./_global.hpp"

static void LoadConfig() {
    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(ConfigTcpBindAddress, "TcpBindAddress");
    CL.Optional(ConfigExportTcpBindAddress, "ExportTcpBindAddress");
    CL.Require(ConfigServerListDownloadAddress, "ServerListDownloadAddress");
    CL.Optional(ConfigUdpMapping, "UdpMapping");
}

static void ParseIpAuthConfig() {
    ConfigExportTcpBindAddress = Trim(ConfigExportTcpBindAddress);
    if (ConfigExportTcpBindAddress.empty()) {
        return;
    }
    ConfigExportTcpBindAddress = Trim(ConfigExportTcpBindAddress);
    auto TA                    = xNetAddress::Parse(ConfigExportTcpBindAddress);
    if (!TA || !TA.Port) {
        Logger->E("Invalid ExportTcpBindAddress");
        QuickExit();
    }
    ConfigExportTcpBindAddress = TA.ToString();  // normalization
}

static void ParseUdpMappingConfig() {
    RuntimeAssert(UdpMappingList.empty());
    auto Segs = Split(Trim(ConfigUdpMapping), ",");
    for (auto P : Segs) {
        P = Trim(P);
        if (P.empty()) {
            continue;  // ignore empty segs
        }
        auto SS = Split(P, "->");
        if (SS.size() != 2) {
            Logger->I("Ignore invalid udp mapping segsment: %s", P.c_str());
            continue;
        }
        auto S = xNetAddress::Parse(SS[0]);
        auto D = xNetAddress::Parse(SS[1]);
        if (!S || !D || S.Port || D.Port) {
            Logger->E("Invalid udp mapping addresses");
            xel::QuickExit();
        }
        UdpMappingList.push_back(std::make_pair(S, D));
    }
    for (auto & P : UdpMappingList) {
        Logger->I("Udp mapping: %s -> %s", P.first.IpToString().c_str(), P.second.IpToString().c_str());
    }
}

static void ParseAndValidateConfig() {
    ParseIpAuthConfig();
    ParseUdpMappingConfig();
}

int main(int argc, char ** argv) {
    X_VAR xRuntimeEnvGuard(argc, argv);
    Logger->I("Starting new program instance");

    LoadConfig();
    ParseAndValidateConfig();

    X_SCOPE(InitClientManager, CleanClietManager);
    X_SCOPE(InitRelayConnectionManager, CleanRelayConnectionManager);
    X_SCOPE(InitAuthCacheLocalServer, CleanAuthCacheLocalServer);
    X_SCOPE(InitDeviceSelector, CleanDeviceSelector);

    while (ServiceRunState) {
        ServiceUpdateOnce(TickClientManager, TickRelayConnectionManager, TickAuthCacheLocalServer, TickDeviceSelector);
    }

    Logger->I("Program instance finished");
    return 0;
}