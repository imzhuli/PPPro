#include "./_global.hpp"

static auto ServerIdClient       = xServerIdClient();
static auto RegisterServerClient = xRegisterServerClient();

static void DSD_RegisterServer(xMessagePoster * Poster, uint64_t LocalServerId) {
    auto Req             = xPP_RegisterDeviceSelectorDispatcher();
    Req.ServerId         = LocalServerId;
    Req.AddressForClient = ExportBindAddressForClient;
    Req.AddressForServer = ExportBindAddressForServer;
    Poster->PostMessage(Cmd_RegisterDeviceSelectorDispatcherServer, 0, Req);
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

    ServerIdClient.SetCallback([](auto ServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, ServerId);
        RegisterServerClient.SetLocalServerId(ServerId);
    });
    RegisterServerClient.SetServerIdPoster(DSD_RegisterServer);

    while (ServiceRunState) {
        ServiceUpdateOnce(ClientSideService, ServerSideService, ServerIdClient, RegisterServerClient);
    }

    return 0;
}
