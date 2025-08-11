#include "../lib_backend_connection/backend_connection_pool.hpp"
#include "./_global.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>

// struct xAuthTest : public xBackendConnectionPool {

//     using xBackendConnectionPool::Clean;
//     using xBackendConnectionPool::Init;

//     bool OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
//         switch (CommandId) {
//             case Cmd_BackendAuthByUserPassResp:
//                 return OnCmdAuthByUserPassResp(CommandId, RequestId, PayloadPtr, PayloadSize);

//             default:
//                 X_DEBUG_PRINTF("unsupported protocol command");
//                 break;
//         }
//         return true;
//     }

//     bool OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
//         auto P = xPPB_BackendAuthByUserPassResp();
//         if (!P.Deserialize(PayloadPtr, PayloadSize)) {
//             X_DEBUG_PRINTF("invalid protocol");
//             return false;
//         }
//         X_DEBUG_PRINTF("%s", P.ToString().c_str());

//         return true;
//     }

//     //
// };

//////////

auto ServerIdClient              = xServerIdClient();
auto RegisterServerClient        = xRegisterServerClient();
auto BackendServerListDownloader = xAC_BackendServerListDownloader();
auto AuthService                 = xAC_AuthService();

auto TA = xNetAddress::Parse("45.202.204.29:20005");

using namespace std::chrono_literals;

// static void Test() {
//     static xTimer TestTimer;
//     if (TestTimer.TestAndTag(1s)) {
//         auto T     = xPPB_BackendAuthByUserPass();
//         T.UserPass = "C_he_0_US_1001__5_78758832:1234567";
//         T.ClientIp = xNetAddress::Parse("45.202.204.29:7777");
//         C.PostMessage(Cmd_BackendAuthByUserPass, 0, T);
//     }
// }

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

    BackendServerListDownloader.SetOnUpdateCallback([](uint32_t V, auto & FL, auto & AL, auto & RL) {
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
    });

    RegisterServerClient.SetServerIdPoster(&AC_RegisterServer);

    ServerIdClient.SetOnServerIdUpdateCallback([](uint64_t LocalServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, LocalServerId);
        RegisterServerClient.SetLocalServerId(LocalServerId);
    });

    while (true) {
        ServiceUpdateOnce(ServerIdClient, RegisterServerClient, BackendServerListDownloader, AuthService);
    }

    return 0;
    //
}
