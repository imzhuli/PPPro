#include "../lib_server_util/all.hpp"
#include "./config.hpp"

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");

    while (ServiceRunState) {
        ServiceUpdateOnce();
    }

    return 0;
}