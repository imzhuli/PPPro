#include "../lib_utils/all.hpp"
#include "./_global.hpp"

static auto TcpServer = xTcpServer();

int main(int argc, char ** argv) {
    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ConfigTcpBindAddress, "TcpBindAddress");
    CL.Require(ConfigServerListDownloadAddress, "ServerListDownloadAddress");

    X_GUARD(AuditAccountServerListDownloader, ServiceIoContext, ConfigServerListDownloadAddress);
    X_GUARD(AuthCacheServerListDownloader, ServiceIoContext, ConfigServerListDownloadAddress);
    X_GUARD(TcpServer, ServiceIoContext, ConfigTcpBindAddress, &ClientConnectionManager);

    AuditAccountServerListDownloader.OnUpdateAuditAccountServerListCallback = [](uint32_t Version, const std::vector<xServerInfo> & List) {
        for (auto & I : List) {
            Logger->I("AA_ServerId=%" PRIi64 ", Address=%s", I.ServerId, I.Address.ToString().c_str());
        }
        // TODO
    };
    AuthCacheServerListDownloader.OnUpdateAuthCacheServerListCallback = [](uint32_t Version, const std::vector<xServerInfo> & List) {
        for (auto & I : List) {
            Logger->I("AC_ServerId=%" PRIi64 ", Address=%s", I.ServerId, I.Address.ToString().c_str());
        }
        // TODO
    };

    while (true) {
        ServiceUpdateOnce(
            AuditAccountServerListDownloader,  //
            AuthCacheServerListDownloader,     //
            ClientConnectionManager,           //
            DeadTicker
        );
    }

    return 0;
}
