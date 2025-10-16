#include "../lib_utils/all.hpp"
#include "./_global.hpp"

int main(int argc, char ** argv) {
    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ConfigTcpBindAddress, "TcpBindAddress");
    CL.Require(ConfigUdpBindAddress, "UdpBindAddress");
    CL.Require(ConfigExportUdpServerAddress, "ExportUdpServerAddress");
    CL.Require(ConfigServerListDownloadAddress, "ServerListDownloadAddress");

    X_SCOPE(InitClientManager, CleanClietManager);
    X_SCOPE(InitAuditAccountService, CleanAuditAccountService);
    X_SCOPE(InitAuthCacheLocalServer, CleanAuthCacheLocalServer);
    X_SCOPE(InitDeviceSelector, CleanDeviceSelector);
    X_SCOPE(InitRelayConnectionManager, CleanRelayConnectionManager);

    auto AATicker  = xTickRunner(TickAuditAccountService);
    auto CMTicker  = xTickRunner(TickClientManager);
    auto ACLTicker = xTickRunner(TickAuthCacheLocalServer);
    auto DSTicker  = xTickRunner(TickDeviceSelector);
    auto RLMTicker = xTickRunner(TickRelayConnectionManager);
    while (true) {
        ServiceUpdateOnce(AATicker, CMTicker, ACLTicker, DSTicker, RLMTicker);
    }

    return 0;
}
