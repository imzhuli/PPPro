#include "../lib_server_util/server_id_client.hpp"

#include <pp_common/base.hpp>

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);
auto GT  = xTicker();

class xTestServerIdClient : public xServerIdClient {
    void OnServerIdUpdated(uint64_t NewServerId) override { X_PDEBUG("NewServerId=%" PRIx64 ", (%" PRIu64 ")", NewServerId, NewServerId); };
};

int main(int argc, char ** argv) {

    auto CL = xCommandLine(
        argc, argv,
        {
            { 'a', nullptr, "config-center-address", true },
            { 'p', nullptr, "previous-server-id", true },
        }
    );

    auto AllocatorAddress    = xNetAddress::Parse("127.0.0.1", 7777);
    auto AllocatorAddressOpt = CL["config-center-address"];
    if (AllocatorAddressOpt) {
        AllocatorAddress = xNetAddress::Parse(*AllocatorAddressOpt);
        if (!AllocatorAddress) {
            X_PFATAL("Invalid address: %s", AllocatorAddressOpt->c_str());
        }
    }
    uint64_t PreviousServerId    = 0;
    auto     PreviousServerIdOpt = CL["previous-server-id"];
    if (PreviousServerIdOpt) {
        PreviousServerId = (uint64_t)std::strtoull(PreviousServerIdOpt->c_str(), nullptr, 10);
    }

    auto TSIC  = xTestServerIdClient();
    auto TSICG = xResourceGuard(TSIC, &IC, AllocatorAddress, PreviousServerId);
    X_RUNTIME_ASSERT(TSICG);

    while (true) {
        GT.Update();
        IC.LoopOnce();
        TSIC.Tick(GT());
    }

    return 0;
}
