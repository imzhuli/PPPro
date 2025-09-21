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

    CL.Require(ConfigTcpBindAddress4, "TcpBindAddress");
    CL.Require(ConfigUdpBindAddress4, "UdpBindAddress");
    CL.Require(ConfigExportUdpServerAddress4, "ExportUdpServerAddress");
    CL.Require(ConfigServerListDownloadAddress, "ServerListDownloadAddress");

    X_GUARD(AuditAccountServerListDownloader, ServiceIoContext, ConfigServerListDownloadAddress);
    X_GUARD(AuthClient, ServiceIoContext, ConfigServerListDownloadAddress);

    AuditAccountServerListDownloader.OnUpdateAuditAccountServerListCallback = [](uint32_t Version, const std::vector<xServerInfo> & List) {
        for (auto & I : List) {
            Logger->I("AA_ServerId=%" PRIi64 ", Address=%s", I.ServerId, I.Address.ToString().c_str());
        }
        // TODO
    };

    auto CMTicker = xTickRunner(ClientManagerTick);
    while (true) {
        ServiceUpdateOnce(
            AuditAccountServerListDownloader,  //
            AuthClient,                        //
            CMTicker
        );
    }

    return 0;
}
