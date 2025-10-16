#include "../lib_backend_connection/backend_connection_pool.hpp"
#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>

int main(int argc, char ** argv) {

    X_VAR xRuntimeEnvGuard(argc, argv, false);

    auto BC = xBackendConnectionPool();
    X_GUARD(BC, ServiceIoContext, 100);

    BC.AddServer(xNetAddress::Parse("127.0.0.1:7000"), "Hello", "World");
    while (true) {
        ServiceUpdateOnce(BC);
    }

    return 0;
}