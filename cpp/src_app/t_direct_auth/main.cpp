#include "../lib_backend_connection/backend_connection_pool.hpp"

#include <pp_common/_.hpp>

auto IC  = xel::xIoContext();
auto ICG = xel::xResourceGuard(IC);
auto BC  = xBackendConnectionPool();
auto BCG = xResourceGuard(BC, &IC, 10);
auto RS  = xel::xRunState();

int main(int argc, char ** argv) {
    auto CL = xel::xCommandLine(
        argc, argv,
        {
            { 'a', nullptr, "address", true },
            { 'u', nullptr, "username", true },
            { 'p', nullptr, "password", true },
        }
    );

    auto OA = CL["address"];
    auto OU = CL["username"];
    auto OP = CL["password"];

    if (!OA || !OU || !OP) {
    }

    RS.Start();
    while (RS) {
        IC.LoopOnce();
    }

    return 0;
}