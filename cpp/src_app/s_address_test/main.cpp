#include "../lib_utils/all.hpp"

struct xTestAddressServer : xService {

    void OnClientConnected(xServiceClientConnection & Connection) override {
        auto PeerAddress = Connection.GetRemoteAddress();
        Logger->I("NewConnectin: Id=%" PRIx64 ", Address=%s", Connection.GetConnectionId(), PeerAddress.ToString().c_str());

        auto Push              = xPP_AddressAutoPush();
        Push.ConnectionAddress = PeerAddress;

        PostMessage(Connection, Cmd_DV_RL_AddressPush, 0, Push);
        DeferKillConnection(Connection);
    }

    //
};

struct xTestAddressUdpServer : xUdpService {
    void OnPacket(const xNetAddress & RemoteAddress, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        if (CommandId != Cmd_DV_RL_AddressChallenge) {
            return;
        }

        auto Push              = xPP_AddressAutoPush();
        Push.ConnectionAddress = RemoteAddress;

        PostMessage(RemoteAddress, Cmd_DV_RL_AddressPush, 0, Push);
        //
    }

    //
};

static xTestAddressServer    TestAddressServer4;
static xTestAddressServer    TestAddressServer6;
static xTestAddressUdpServer TestAddressUdpServer4;
static xTestAddressUdpServer TestAddressUdpServer6;

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    auto BindAddress4 = xNetAddress();
    auto BindAddress6 = xNetAddress();

    CL.Require(BindAddress4, "BindAddress4");
    CL.Require(BindAddress6, "BindAddress6");

    auto V4Enabled = (bool)BindAddress4;
    auto V6Enabled = (bool)BindAddress6;
    if (!V4Enabled && !V6Enabled) {
        Logger->F("no available bind address");
    }

    X_COND_GUARD(V4Enabled, TestAddressServer4, ServiceIoContext, BindAddress4, 1024);
    X_COND_GUARD(V4Enabled, TestAddressUdpServer4, ServiceIoContext, BindAddress4);
    X_COND_GUARD(V6Enabled, TestAddressServer6, ServiceIoContext, BindAddress6, 1024);
    X_COND_GUARD(V6Enabled, TestAddressUdpServer6, ServiceIoContext, BindAddress6);

    while (ServiceRunState) {
        ServiceUpdateOnce();
        if (V4Enabled) {
            TestAddressServer4.Tick(ServiceTicker());
        }
        if (V6Enabled) {
            TestAddressServer6.Tick(ServiceTicker());
        }
    }

    return 0;
    //
}
