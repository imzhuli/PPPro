#include "./challenge_service.hpp"

#include "./_global.hpp"

void OnTerminalChallenge(const xUdpServiceChannelHandle & Handle, xPacketCommandId CmdId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CmdId != Cmd_DV_CC_Challenge) {
        Logger->E("invalid command id");
        return;
    }

    // todo: read input
    auto DC = xCC_DeviceChallenge();
    if (!DC.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("invalid protocol");
        return;
    }

    auto RelayAddress = xNetAddress();
    if (Handle.GetRemoteAddress().IsV4()) {
        auto SI = GetRandomDeviceRelayServer4();
        if (SI) {
            RelayAddress = SI->ExportDeviceAddress4;
        }
    } else if (Handle.GetRemoteAddress().IsV6()) {
        auto SI = GetRandomDeviceRelayServer6();
        if (SI) {
            RelayAddress = SI->ExportDeviceAddress6;
        }
    }

    auto Resp = xCC_DeviceChallengeResp();
    if (RelayAddress) {
        DEBUG_LOG("relay server found: %s", RelayAddress.ToString().c_str());
        auto Key = RelayAddress.ToString();

        Resp.Accepted      = true;
        Resp.RelayAddress  = RelayAddress;
        Resp.RelayCheckKey = Key;
    } else {
        DEBUG_LOG("no relay server found");
    }

    Handle.PostMessage(Cmd_DV_CC_ChallengeResp, RequestId, Resp);
}
