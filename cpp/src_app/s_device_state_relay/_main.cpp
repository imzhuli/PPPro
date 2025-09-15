#include "../lib_utils/all.hpp"
#include "./_global.hpp"

#include <pp_common/_.hpp>
#include <pp_protocol/command.hpp>

using namespace xel;

class xObserverService;
class xProducerService;

static std::vector<std::unique_ptr<xTcpServiceClientConnectionHandle>> Connections;
static xTcpService                                                     OS;  // observer
static xTcpService                                                     PS;  // producer

static void DispatchData(const void * DataPtr, size_t DataSize) {
    DEBUG_LOG("Dispatching data:\n%s", HexShow(DataPtr, DataSize).c_str());
    for (auto & H : Connections) {
        DEBUG_LOG("ToConnection:%" PRIx64 "", H->GetConnectionId());
        H->PostData(DataPtr, DataSize);
    }
}

static auto ServiceGuard = xScopeGuard(
    [] {
        OS.OnClientConnected = [](const xTcpServiceClientConnectionHandle & H) { Connections.push_back(std::make_unique<xTcpServiceClientConnectionHandle>(H)); };
        OS.OnClientClose     = [](const xTcpServiceClientConnectionHandle & H) {
            auto I = Connections.begin();
            auto E = Connections.end();
            auto T = H.operator->();
            while (I != E) {
                if (I->get()->operator->() == T) {
                    Connections.erase(I);
                    break;
                }
                ++I;
            }
        };
        PS.OnClientPacket = [](const xTcpServiceClientConnectionHandle Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
            DEBUG_LOG("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
            switch (CommandId) {
                case Cmd_DSR_DS_DeviceUpdate: {
                    ubyte  B[MaxPacketSize];
                    size_t RS = BuildPacket(B, CommandId, 0, PayloadPtr, PayloadSize);
                    assert(RS);
                    DispatchData(B, RS);
                } break;
                default: {
                    DEBUG_LOG("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
                } break;
            }
            return true;
        };
    },
    xPass()
);

static void DSR_RegisterServer(const xMessageChannel & Poster, uint64_t LocalServerId) {
    auto Req            = xPP_RegisterDeviceStateRelayServer();
    Req.ServerId        = LocalServerId;
    Req.Address         = ExportProducerAddress;
    Req.ObserverAddress = ExportObserverAddress;
    Poster.PostMessage(Cmd_RegisterDeviceStateRelayServer, 0, Req);
}

static xServerIdClient       ServerIdClient;
static xRegisterServerClient RegisterServerClient;

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");
    CL.Require(BindObserverAddress, "BindObserverAddress");
    CL.Require(BindProducerAddress, "BindProducerAddress");
    CL.Require(ExportObserverAddress, "ExportObserverAddress");
    CL.Require(ExportProducerAddress, "ExportProducerAddress");

    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RegisterServerClient, ServiceIoContext, ServerListRegisterAddress);

    ServerIdClient.OnServerIdUpdateCallback = [](auto ServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, ServerId);
        RegisterServerClient.SetLocalServerId(ServerId);
    };

    RegisterServerClient.SetServerIdPoster(&DSR_RegisterServer);

    X_GUARD(OS, ServiceIoContext, BindObserverAddress, DEFAULT_MAX_SERVER_CONNECTIONS);
    X_GUARD(PS, ServiceIoContext, BindProducerAddress, DEFAULT_MAX_SERVER_CONNECTIONS);

    while (true) {
        ServiceUpdateOnce(OS, PS, ServerIdClient, RegisterServerClient);
    }

    return 0;
}
