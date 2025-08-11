#include "./device_state_relay_server_list_downloader.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xDeviceStateRelayServerListDownloader::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (NowMS - LastUpdateTimestampMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostDownloadDeviceStateRelayServerListRequest();
        LastUpdateTimestampMS = Ticker();
        return;
    }
}

void xDeviceStateRelayServerListDownloader::OnServerConnected() {
    PostDownloadDeviceStateRelayServerListRequest();
    LastUpdateTimestampMS = Ticker();
}

bool xDeviceStateRelayServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    switch (CommandId) {
        case Cmd_DownloadDeviceStateRelayServerListResp:
            return OnDeviceStateRelayServerList(RequestId, PayloadPtr, PayloadSize);
        default:
            return false;
    }

    return true;
}

bool xDeviceStateRelayServerListDownloader::OnDeviceStateRelayServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    // DEBUG_LOG("%s", HexShow(PayloadPtr, PayloadSize).c_str());

    auto R = xPP_DownloadDeviceStateRelayServerListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (DeviceStateRelayServerListVersion == R.Version) {
        return true;
    }
    auto & NewServerList = R.ServerInfoList;
    std::sort(NewServerList.begin(), NewServerList.end(), [](auto & lhs, auto & rhs) { return lhs.ServerId < rhs.ServerId; });

    DeviceStateRelaySortedServerInfoList = std::move(R.ServerInfoList);
    DeviceStateRelayServerListVersion    = R.Version;

    UpdateDeviceStateRelayServerListCallback(DeviceStateRelayServerListVersion, DeviceStateRelaySortedServerInfoList);
    return true;
}

void xDeviceStateRelayServerListDownloader::PostDownloadDeviceStateRelayServerListRequest() {
    auto R    = xPP_DownloadDeviceStateRelayServerList();
    R.Version = DeviceStateRelayServerListVersion;
    PostMessage(Cmd_DownloadDeviceStateRelayServerList, 0, R);
}
