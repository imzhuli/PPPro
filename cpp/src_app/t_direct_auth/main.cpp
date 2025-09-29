#include "../lib_client/auth_client.hpp"
#include "./class.hpp"

#include <pp_common/_.hpp>

auto IC  = xel::xIoContext();
auto ICG = xel::xResourceGuard(IC);
auto AC  = xAuthClient();

auto TestAccount = "C_he_BR_1001__5_78758832:1234567";

auto Ticker    = xTicker();
auto StartTime = Ticker();
auto Request   = false;

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv, false);
    auto CL  = xel::xCommandLine(
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

    if (!OA) {
        cerr << "invalid params" << endl;
        return -1;
    }

    X_GUARD(AC, ServiceIoContext, xNetAddress::Parse(*OA));
    AC.OnEnabled = []() {
        cout << "enabled" << endl;
        if (!AC.Request(time(nullptr), TestAccount)) {
            cerr << "failed to post auth request" << endl;
        }
    };
    AC.OnAuthCacheResultCallback = [](uint64_t RequestContextId, const xClientAuthResult * AuthResult) {
        cout << "RequestId: " << RequestContextId << endl;
        X_AT_EXIT([] { ServiceRunState.Stop(); });

        if (!AuthResult) {
            cout << "no result" << endl;
            return;
        }
        cout << "AuditId: " << AuthResult->AuditId << endl;
        return;
    };

    while (ServiceRunState) {
        ServiceUpdateOnce(AC);
    }

    return 0;
}