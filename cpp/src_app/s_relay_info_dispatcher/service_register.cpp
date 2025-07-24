#include "./service_register.hpp"

#include "./global.hpp"

static void RID_RegisterServer(xMessagePoster * Poster, uint64_t LocalServerId) {
    assert(LocalServerId);
    auto Req       = xPP_RegisterRelayInfoDispatcherServer();
    Req.ServerInfo = {
        .ServerId        = LocalServerId,
        .ProducerAddress = ExportProducerAddress,
        .ObserverAddress = ExportObserverAddress,
    };
    Poster->PostMessage(Cmd_RegisterRelayInfoDispatcherServer, 0, Req);
}

bool xRID_RegisterServerService::Init() {
    RuntimeAssert(ServerIdClient.Init(ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath));
    RuntimeAssert(RegisterClient.Init(ServiceIoContext, ServerListRegisterAddress));

    ServerIdClient.SetCallback([this](uint64_t NewServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, NewServerId);
        RegisterClient.SetLocalServerId(NewServerId);
    });
    RegisterClient.SetServerIdPoster(RID_RegisterServer);
    return true;
}

void xRID_RegisterServerService::Clean() {
    ServerIdClient.Clean();
    RegisterClient.Clean();
}

void xRID_RegisterServerService::Tick(uint64_t NowMS) {
    TickAll(NowMS, ServerIdClient, RegisterClient);
}
