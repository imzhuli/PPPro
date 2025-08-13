#include "../lib_utils/all.hpp"

#include <pp_protocol/internal/server_id.hpp>

static constexpr const size_t   MAX_ID_INDEX                 = 10'0000;
static constexpr const uint64_t SERVER_ID_RECYCLE_TIMEOUT_MS = 3 * 1'000;

static xCollectableErrorPrinter ErrorPrinter = { "Failed to allocate server id" };
static xServerIdManager         ServerIdManager;
static xNetAddress              BindAddress4;
static xNetAddress              BindAddress6;
static xTcpServiceWrapper       Ipv4Service;
static bool                     Ipv4Enabled = false;
static xTcpServiceWrapper       Ipv6Service;
static bool                     Ipv6Enabled = false;

static bool OnClientPacket(const xTcpServiceMessageChannel & Channel, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto   Connection = static_cast<xServiceClientConnection *>(Channel.GetUnderLayeredObjectUnchecked());
    auto & ServerId   = Connection->UserContext.U64;
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
    Channel.PostMessageUnchecked(Cmd_AcquireServerIdResp, RequestId, Resp);
    return true;
}

static void OnClientClose(const xTcpServiceMessageChannel & Poster) {
    auto   Connection = static_cast<xServiceClientConnection *>(Poster.GetUnderLayeredObjectUnchecked());
    auto & ServerId   = Connection->UserContext.U64;
    if (ServerId) {
        Logger->I("Releasing ServerId: %" PRIu64 "", ServerId);
        X_RUNTIME_ASSERT(ServerIdManager.ReleaseServerId(ServerId));
    }
}

int main(int argc, char ** argv) {

    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(BindAddress4, "BindAddress4");
    CL.Require(BindAddress6, "BindAddress6");

    ErrorPrinter.SetLogger(Logger);

    X_GUARD(ServerIdManager);
    if (BindAddress4.IsV4() && BindAddress4.Port) {
        Ipv4Enabled = true;
        Ipv4Service.SetOnClientPacketCallback(OnClientPacket);
        Ipv4Service.SetOnClientCloseCallback(OnClientClose);
    }
    if (BindAddress6.IsV6() && BindAddress6.Port) {
        Ipv6Enabled = true;
        Ipv6Service.SetOnClientPacketCallback(OnClientPacket);
        Ipv6Service.SetOnClientCloseCallback(OnClientClose);
    }
    if (!Ipv4Enabled && !Ipv6Enabled) {
        Logger->F("No bind address enabled");
    }

    X_COND_GUARD(Ipv4Enabled, Ipv4Service, ServiceIoContext, BindAddress4, MAX_ID_INDEX);
    X_COND_GUARD(Ipv6Enabled, Ipv6Service, ServiceIoContext, BindAddress6, MAX_ID_INDEX);

    while (ServiceRunState) {
        ServiceUpdateOnce();
        if (Ipv4Enabled) {
            Ipv4Service.Tick(ServiceTicker());
        }
        if (Ipv6Enabled) {
            Ipv6Service.Tick(ServiceTicker());
        }
    }

    return 0;
}
