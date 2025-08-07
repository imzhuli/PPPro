#include "../lib_utils/all.hpp"

struct xTestAddressServer : xService {

    void OnClientConnected(xServiceClientConnection & Connection) override {
        auto PeerAddress = Connection.GetRemoteAddress();
        Logger->I("NewConnectin: Id=%" PRIx64 ", Address=%s", Connection.GetConnectionId(), PeerAddress.ToString().c_str());

        auto Push              = xPP_AddressAutoPush();
        Push.ConnectionAddress = PeerAddress;
        DeferKillConnection(Connection);
    }

    //
};
static xTestAddressServer TestAddressServer;

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    auto BindAddress = xNetAddress();
    CL.Require(BindAddress, "BindAddress");

    X_GUARD(TestAddressServer, ServiceIoContext, BindAddress, 1024);
    auto TS = xTestAddressServer();

    while (ServiceRunState) {
        ServiceUpdateOnce();
    }

    return 0;
    //
}
