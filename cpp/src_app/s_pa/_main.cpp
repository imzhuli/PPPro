#include "../lib_utils/all.hpp"
#include "./_global.hpp"

static auto TcpServer = xTcpServer();

// static bool EnableTcp4 = false;
// static bool EnableTcp6 = false;
// static bool EnableUdp4 = false;
// static bool EnableUdp6 = false;

int main(int argc, char ** argv) {
    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ConfigTcpBindAddress, "TcpBindAddress");
    CL.Require(ConfigUdpBindAddress, "UdpBindAddress");
    CL.Require(ConfigExportUdpServerAddress, "ExportUdpServerAddress");
    CL.Require(ConfigServerListDownloadAddress, "ServerListDownloadAddress");

    auto X_VAR = xel::xScopeGuard(InitClientManager, CleanClietManager);
    auto X_VAR = xel::xScopeGuard(InitAuditAccountService, CleanAuditAccountService);

    X_GUARD(AuthClient, ServiceIoContext, ConfigServerListDownloadAddress);

    auto AATicker = xTickRunner(TickAuditAccountService);
    auto CMTicker = xTickRunner(TickClientManager);
    while (true) {
        ServiceUpdateOnce(
            AuthClient,  //
            AATicker, CMTicker
        );
    }

    return 0;
}
