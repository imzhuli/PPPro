#include "../lib_backend_connection/backend_connection_pool.hpp"
#include "./_global.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>

//////////

auto TA = xNetAddress::Parse("45.202.204.29:20005");

using namespace std::chrono_literals;

static void AC_RegisterServer(const xMessageChannel & Poster, uint64_t LocalServerId) {
    auto Req     = xPP_RegisterAuthCacheServer();
    Req.ServerId = LocalServerId;
    Req.Address  = ExportServerAddress;
    Poster.PostMessage(Cmd_RegisterAuthCacheServer, 0, Req);
}

int main(int argc, char ** argv) {
    auto Env = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();
    CL.Require(BindAddress, "BindAddress");
    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");
    CL.Require(ExportServerAddress, "ExportServerAddress");
    CL.Require(BackendServerAppKey, "BackendServerAppKey");
    CL.Require(BackendServerAppSecret, "BackendServerAppSecret");

    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RegisterServerClient, ServiceIoContext, ServerListRegisterAddress);
    X_GUARD(BackendServerListDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(AuthService, ServiceIoContext, BindAddress);

    AuthService.UpdateBackendAuthInfo(BackendServerAppKey, BackendServerAppSecret);

    BackendServerListDownloader.OnUpdateCallback = [](uint32_t V, auto & FL, auto & AL, auto & RL) {
        auto OS = std::ostringstream();
        OS << "BackendServerListVersion: " << V << endl;
        OS << "Added: " << endl;
        for (auto & A : AL) {
            OS << A.ToString() << endl;
        }
        OS << "Removed: " << endl;
        for (auto & R : RL) {
            OS << R.ToString() << endl;
        }
        OS << "BackendServerList Updated";
        Logger->I("BackendServerListDownloaded: %s", OS.str().c_str());
        AuthService.UpdateBackendServerList(AL, RL);
    };

    RegisterServerClient.ServerRegister = &AC_RegisterServer;

    ServerIdClient.OnServerIdUpdateCallback = [](uint64_t LocalServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, LocalServerId);
        RegisterServerClient.SetLocalServerId(LocalServerId);
    };

    while (true) {
        ServiceUpdateOnce(ServerIdClient, RegisterServerClient, BackendServerListDownloader, AuthService);
    }

    return 0;
    //
}
