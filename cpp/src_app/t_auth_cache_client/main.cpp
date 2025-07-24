#include "../lib_server_util/all.hpp"

struct xTestAuthClient : xClient {

    void OnServerConnected() override {
        DEBUG_LOG("Server connected");

        auto R     = xPP_QueryAuthCache();
        R.UserPass = "E_he_0_BR___10_0_lk85:123456";

        PostMessage(Cmd_AuthService_QueryAuthCache, 0, R);
        //
    }

    void OnServerClose() override {
        DEBUG_LOG("");
        //
    }

    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        DEBUG_LOG("%s", HexShow(PayloadPtr, PayloadSize).c_str());
        ServiceRunState.Stop();
        return true;
    }

    //
};

xTestAuthClient Client;
xNetAddress     AuthCacheServerAddress;

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(AuthCacheServerAddress, "AuthCacheServerAddress");

    auto CG = xResourceGuard(Client, ServiceIoContext, AuthCacheServerAddress);
    RuntimeAssert(CG);

    while (ServiceRunState) {
        ServiceUpdateOnce(Client);
    }

    return 0;
}