#include "../lib_server_util/all.hpp"
#include "./_global.hpp"

static auto TcpServer = xTcpServer();

int main(int argc, char ** argv) {
    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ConfigTcpBindAddress, "TcpBindAddress");
    CL.Require(ConfigServerListDownloadAddress, "ServerListDownloadAddress");

    X_GUARD(AuthCacheLocalServer, ServiceIoContext);
    X_GUARD(ClientConnectionManager, ServiceIoContext);
    X_GUARD(AuthCacheServerListDownloader, ServiceIoContext, ConfigServerListDownloadAddress);
    X_GUARD(TcpServer, ServiceIoContext, ConfigTcpBindAddress, &ClientConnectionManager);

    AuthCacheServerListDownloader.SetUpdateAuthCacheServerListCallback([](const std::vector<xServerInfo> & List) {
        for (auto & I : List) {
            Logger->I("ServerId=%" PRIi64 ", Address=%s", I.ServerId, I.Address.ToString().c_str());
        }
        AuthCacheLocalServer.UpdateServerList(List);
    });
    AuthCacheLocalServer.SetCallback([](uint64_t RequestContextId, const xClientAuthResult & AuthResult) {
        ClientConnectionManager.OnAuthResult(RequestContextId, AuthResult);  // relay result
    });

    while (true) {
        ServiceUpdateOnce(AuthCacheServerListDownloader, ClientConnectionManager, AuthCacheLocalServer);
    }

    return 0;
}
