#include "../lib_utils/all.hpp"
#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <server_arch/service.hpp>

using namespace xel;

class xObserverService;
class xProducerService;

class xObserverService : xService {
public:
    using xService::Init;
    void Clean() {
        xService::Clean();
        Reset(Connections);
    }
    using xService::Tick;

    void OnClientConnected(xServiceClientConnection & Connection) override { Connections.push_back(&Connection); }

    void OnClientClose(xServiceClientConnection & Connection) override {
        auto I = Connections.begin();
        auto E = Connections.end();
        while (I != E) {
            if (*I == &Connection) {
                Connections.erase(I);
                break;
            }
            ++I;
        }
    }

    void DispatchData(const void * DataPtr, size_t DataSize) {
        DEBUG_LOG("Dispatching data:\n%s", HexShow(DataPtr, DataSize).c_str());
        for (auto PC : Connections) {
            DEBUG_LOG("ToConnection:%" PRIx64 "", PC->GetConnectionId());
            PC->PostData(DataPtr, DataSize);
        }
    }

private:
    std::vector<xServiceClientConnection *> Connections;

    //
};

static xObserverService OS;

static void DSR_RegisterServer(const xMessagePoster & Poster, uint64_t LocalServerId) {
    auto Req            = xPP_RegisterDeviceStateRelayServer();
    Req.ServerId        = LocalServerId;
    Req.Address         = ExportProducerAddress;
    Req.ObserverAddress = ExportObserverAddress;
    Poster.PostMessage(Cmd_RegisterDeviceStateRelayServer, 0, Req);
}

class xProducerService : xService {
public:
    using xService::Clean;
    using xService::Init;
    using xService::Tick;

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        DEBUG_LOG("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
        switch (CommandId) {
            case Cmd_DSR_DS_DeviceUpdate: {
                ubyte  B[MaxPacketSize];
                size_t RS = BuildPacket(B, CommandId, 0, PayloadPtr, PayloadSize);
                assert(RS);
                OS.DispatchData(B, RS);
            } break;

            default: {
                DEBUG_LOG("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
                break;
            }
        }

        return true;
    }

    //
};

static xProducerService      PS;
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

    ServerIdClient.SetCallback([](auto ServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, ServerId);
        RegisterServerClient.SetLocalServerId(ServerId);
    });

    RegisterServerClient.SetServerIdPoster(&DSR_RegisterServer);

    X_GUARD(OS, ServiceIoContext, BindObserverAddress, DEFAULT_MAX_SERVER_CONNECTIONS);
    X_GUARD(PS, ServiceIoContext, BindProducerAddress, DEFAULT_MAX_SERVER_CONNECTIONS);

    while (true) {
        ServiceUpdateOnce(OS, PS, ServerIdClient, RegisterServerClient);
    }

    return 0;
}
