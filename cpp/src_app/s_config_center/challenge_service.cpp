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

    auto Under = DC.Extract();

    auto RelayAddress = xNetAddress();
    auto GeoInfo      = xGeoInfo{};
    if (Under.Tcp4Address.Is4()) {
        GeoInfo = IpLocationManager.GetRegionByIp(Under.Tcp4Address.IpToString().c_str());
        auto SI = GetRandomDeviceRelayServer4();
        if (SI) {
            RelayAddress = SI->ExportDeviceAddress4;
        }
    }

    if (!RelayAddress && Under.Tcp6Address.Is6()) {
        auto SI = GetRandomDeviceRelayServer6();
        if (!GeoInfo.CountryId) {
            GeoInfo = IpLocationManager.GetRegionByIp(Under.Tcp4Address.IpToString().c_str());
        }
        if (SI) {
            RelayAddress = SI->ExportDeviceAddress6;
        }
    }

    auto Resp = xCC_DeviceChallengeResp();
    if (RelayAddress) {
        DEBUG_LOG(
            "relay server found: %s, DeviceGeoInfo:%u/%u/%u", RelayAddress.ToString().c_str(), (unsigned)GeoInfo.CountryId, (unsigned)GeoInfo.StateId, (unsigned)GeoInfo.CityId
        );
        auto Key      = RelayAddress.ToString();
        Under.GeoInfo = GeoInfo;

        Resp.Accepted      = true;
        Resp.RelayAddress  = RelayAddress;
        Resp.RelayCheckKey = MakeChallengeKey(Under);
    } else {
        DEBUG_LOG("no relay server found");
    }

    Handle.PostMessage(Cmd_DV_CC_ChallengeResp, RequestId, Resp);
}
