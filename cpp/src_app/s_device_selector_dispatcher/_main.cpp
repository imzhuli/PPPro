#include "./_global.hpp"

static auto ServerIdClient       = xServerIdClient();
static auto RegisterServerClient = xRegisterServerClient();

static void DSD_RegisterServer(const xMessageChannel & Poster, uint64_t LocalServerId) {
    auto   Req                                 = xPP_RegisterDeviceSelectorDispatcher();
    auto & ServerInfo                          = Req.ServerInfo;
    ServerInfo.ServerId                        = LocalServerId;
    ServerInfo.ExportAddressForClient          = ExportBindAddressForClient;
    ServerInfo.ExportAddressForServiceProvider = ExportBindAddressForServer;

    Poster.PostMessage(Cmd_RegisterDeviceSelectorDispatcherServer, 0, Req);
}

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Require(BindAddressForClient, "BindAddressForClient");
    CL.Require(ExportBindAddressForClient, "ExportBindAddressForClient");
    CL.Require(BindAddressForServer, "BindAddressForServer");
    CL.Require(ExportBindAddressForServer, "ExportBindAddressForServer");

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");

    X_GUARD(ClientSideService, ServiceIoContext, BindAddressForClient, 5'000);
    X_GUARD(ServerSideService, ServiceIoContext, BindAddressForServer, 5'000);
    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RegisterServerClient, ServiceIoContext, ServerListRegisterAddress);
    X_GUARD(RequestContextPool, 20'0000);
    X_GUARD(ServiceProviderManager);

    ClientSideService.OnClientPacket = OnDSDClientSidePacket;
    ServerSideService.OnClientPacket = OnDSDServerSidePacket;
    ServerSideService.OnClientClose  = OnDSDServerSideClose;

    ServerIdClient.OnServerIdUpdateCallback = [](auto ServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, ServerId);
        RegisterServerClient.SetLocalServerId(ServerId);
    };
    RegisterServerClient.ServerRegister = DSD_RegisterServer;

    while (ServiceRunState) {
        ServiceUpdateOnce(ClientSideService, ServerSideService, ServerIdClient, RegisterServerClient, RequestContextPool);
        OutputLocalAudit();
    }

    return 0;
}
