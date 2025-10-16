#include "../lib_client/audit_target.hpp"

#include <pp_protocol/internal/audit_target.hpp>

static auto ServerIdClient       = xServerIdClient();
static auto RegisterServerClient = xRegisterServerClient();

static auto BindAddress4   = xNetAddress();
static auto ExportAddress4 = xNetAddress();

static auto ServerIdCenterAddress     = xNetAddress();
static auto ServerListRegisterAddress = xNetAddress();

static void AA_RegisterServer(const xMessageChannel & Poster, uint64_t LocalServerId) {
    auto Req     = xPP_RegisterAuditTargetServer();
    Req.ServerId = LocalServerId;
    Req.Address  = ExportAddress4;

    Poster.PostMessage(Cmd_RegisterAuditTargetServer, 0, Req);
}

static auto AAService = xTcpService();

int main(int argc, char ** argv) {
    X_VAR xRuntimeEnvGuard(argc, argv);
    auto  CL = RuntimeEnv.LoadConfig();
    CL.Require(BindAddress4, "BindAddress4");
    CL.Require(ExportAddress4, "ExportAddress4");

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");

    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RegisterServerClient, ServiceIoContext, ServerListRegisterAddress);

    RegisterServerClient.ServerRegister     = AA_RegisterServer;
    ServerIdClient.OnServerIdUpdateCallback = [](auto ServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, ServerId);
        RegisterServerClient.SetLocalServerId(ServerId);
    };

    while (ServiceRunState) {
        ServiceUpdateOnce(ServerIdClient, RegisterServerClient);
    }

    return 0;
}
