#include "../lib_utils/all.hpp"

#include <pp_protocol/internal/server_id.hpp>

static constexpr const size_t   MAX_ID_INDEX                 = 10'0000;
static constexpr const uint64_t SERVER_ID_RECYCLE_TIMEOUT_MS = 3 * 1'000;

static auto ConfigServiceBindAddress = xNetAddress();

static auto IC  = xIoContext();
static auto ICG = xResourceGuard(IC);

class xServerIdCenterService : public xService {

public:
    bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) {
        if (!ErrorPrinter.Init()) {
            return false;
        }
        if (!ServerIdManager.Init()) {
            ErrorPrinter.Clean();
            return false;
        }
        if (!xService::Init(IoContextPtr, BindAddress, MAX_ID_INDEX, true)) {
            ServerIdManager.Clean();
            ErrorPrinter.Clean();
            return false;
        }
        return true;
    }

    void Clean() {
        xService::Clean();
        ServerIdManager.Clean();
        ErrorPrinter.Clean();
    }

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        auto & ServerId = Connection.UserContext.U64;
        if (ServerId) {
            X_DEBUG_PRINTF("Multiple server id request");
            return false;
        }
        if (CommandId != Cmd_AcquireServerId) {
            X_DEBUG_PRINTF("Unrecognized command");
            return false;
        }
        auto Req = xPP_AcquireServerId();
        if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
            X_DEBUG_PRINTF("Invalid data packet");
            return false;
        }

        ServerId = ServerIdManager.RegainServerId(Req.PreviousServerId);
        if (!ServerId) {
            ErrorPrinter.Hit();
        }
        X_DEBUG_PRINTF("ServerId %" PRIx64 " -> %" PRIx64 "", Req.PreviousServerId, ServerId);

        auto Resp             = xPP_AcquireServerIdResp();
        Resp.PreviousServerId = Req.PreviousServerId;
        Resp.NewServerId      = ServerId;
        PostMessage(Connection, Cmd_AcquireServerIdResp, RequestId, Resp);
        return true;
    }

    void OnClientClose(xServiceClientConnection & Connection) override {
        auto & ServerId = Connection.UserContext.U64;
        if (ServerId) {
            X_DEBUG_PRINTF("Releasing ServerId: %" PRIx64 "", ServerId);
            X_RUNTIME_ASSERT(ServerIdManager.ReleaseServerId(ServerId));
        }
    }

private:
    xServerIdManager         ServerIdManager;
    xCollectableErrorPrinter ErrorPrinter = { "Failed to allocate server id" };
};

static auto Service = xServerIdCenterService();

int main(int argc, char ** argv) {

    auto SEG = xRuntimeEnvGuard(argc, argv);

    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(ConfigServiceBindAddress, "BindAddress");

    auto SG = xResourceGuard(Service, &IC, ConfigServiceBindAddress);
    if (!SG) {
        Logger->E("failed to init service, bind-address=%s", ConfigServiceBindAddress.ToString().c_str());
        return -1;
    } else {
        Logger->I("init service done, bind-address=%s", ConfigServiceBindAddress.ToString().c_str());
    }

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();
        TickAll(ServiceTicker(), Service);
    }

    return 0;
}
