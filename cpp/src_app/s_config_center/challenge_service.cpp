#include "./challenge_service.hpp"

#include "./_global.hpp"

void OnTerminalChallenge(const xUdpServiceChannelHandle & Handle, xPacketCommandId CmdId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CmdId != Cmd_DV_CC_Challenge) {
        return;
    }

    // todo: read input

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
        // todo : generate challenge key:
        auto Key = RelayAddress.ToString();

        Resp.Accepted      = true;
        Resp.RelayAddress  = RelayAddress;
        Resp.RelayCheckKey = Key;
    }

    Handle.PostMessage(Cmd_DV_CC_ChallengeResp, RequestId, Resp);
}
