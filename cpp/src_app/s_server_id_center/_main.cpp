#include "../lib_utils/all.hpp"

#include <pp_protocol/internal/server_id.hpp>

static constexpr const size_t MAX_ID_INDEX = 10'0000;

static xCollectableErrorPrinter ErrorPrinter = { "Failed to allocate server id" };
static xServerIdManager         ServerIdManager;
static xNetAddress              BindAddress4;
static xTcpService              TcpService;

static bool OnClientPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto & ServerId = Handle->UserContext.U64;
    if (ServerId) {
        Logger->E("Multiple server id request");
        return false;
    }
    if (CommandId != Cmd_AcquireServerId) {
        Logger->E("Unrecognized command");
        return false;
    }
    auto Req = xPP_AcquireServerId();
    if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("Invalid data packet");
        return false;
    }

    ServerId = ServerIdManager.RegainServerId(Req.PreviousServerId);
    if (!ServerId) {
        Logger->I("Regain server id failed, invalid previous server id, or server id indexes conflit");
        ErrorPrinter.Hit();
    }
    Logger->I("ServerId %" PRIu64 " -> %" PRIu64 "", Req.PreviousServerId, ServerId);

    auto Resp             = xPP_AcquireServerIdResp();
    Resp.PreviousServerId = Req.PreviousServerId;
    Resp.NewServerId      = ServerId;
    Handle.PostMessage(Cmd_AcquireServerIdResp, RequestId, Resp);
    return true;
}

static void OnClientClose(const xTcpServiceClientConnectionHandle & Handle) {
    auto & ServerId = Handle->UserContext.U64;
    if (ServerId) {
        Logger->I("Releasing ServerId: %" PRIu64 "", ServerId);
        X_RUNTIME_ASSERT(ServerIdManager.ReleaseServerId(ServerId));
    }
}

int main(int argc, char ** argv) {

    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(BindAddress4, "BindAddress4");

    ErrorPrinter.SetLogger(Logger);

    TcpService.OnClientPacket = OnClientPacket;
    TcpService.OnClientClose  = OnClientClose;
    X_GUARD(ServerIdManager);

    X_GUARD(TcpService, ServiceIoContext, BindAddress4, MAX_ID_INDEX);

    while (ServiceRunState) {
        ServiceUpdateOnce();
        TcpService.Tick(ServiceTicker());
    }

    return 0;
}
