#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>

static const xNetAddress BindAddress = xNetAddress::Parse("[::]:9000");

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv, false);

    auto CL = xel::xCommandLine(
        argc, argv,
        {
            { 'r', nullptr, "resp" },
        }
    );

    auto Resp = (bool)CL["resp"];

    xel::xUdpService UdpService;
    UdpService.OnPacketCallback = [Resp](const xUdpServiceChannelHandle & H, xPacketCommandId CID, xPacketRequestId RID, ubyte * PP, size_t PS) {
        printf("UdpServicePacket: Remote=%s CID=%" PRIx32 ", RID=%" PRIx64 "\n%s\n", H.GetRemoteAddress().ToString().c_str(), CID, RID, HexShow(PP, PS).c_str());
        if (Resp) {
            H.PostData(PP, PS);
        }
    };

    X_GUARD(UdpService, ServiceIoContext, BindAddress);

    while (ServiceRunState) {
        ServiceUpdateOnce();
    }

    return 0;
}
