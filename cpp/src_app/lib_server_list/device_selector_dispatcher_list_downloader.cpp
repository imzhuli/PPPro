#include "./device_selector_dispatcher_list_downloader.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

void xDeviceSelectorDispatcherServerListDownloader::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (NowMS - LastUpdateTimestampMS < UpdateTimeoutMS) {
        return;
    }
    if (IsOpen()) {
        PostDeviceSelectorDispatcherServerListRequest();
        LastUpdateTimestampMS = Ticker();
        return;
    }
}

void xDeviceSelectorDispatcherServerListDownloader::OnServerConnected() {
    PostDeviceSelectorDispatcherServerListRequest();
    LastUpdateTimestampMS = Ticker();
}

bool xDeviceSelectorDispatcherServerListDownloader::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    switch (CommandId) {
        case Cmd_DownloadDeviceSelectorDispatcherServerListResp:
            return OnDeviceSelectorDispatcherServerList(RequestId, PayloadPtr, PayloadSize);
        default:
            return false;
    }

    return true;
}

bool xDeviceSelectorDispatcherServerListDownloader::OnDeviceSelectorDispatcherServerList(xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    // DEBUG_LOG("%s", HexShow(PayloadPtr, PayloadSize).c_str());

    auto R = xPP_DownloadDeviceSelectorDispatcherListResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    if (DeviceSelectorDispatcherServerListVersion == R.Version) {
        return true;
    }

    DeviceSelectorDispatcherSortedServerInfoList = std::move(R.ServerInfoList);
    std::sort(DeviceSelectorDispatcherSortedServerInfoList.begin(), DeviceSelectorDispatcherSortedServerInfoList.end(), [](auto & lhs, auto & rhs) {
        return lhs.ServerId < rhs.ServerId;
    });
    DeviceSelectorDispatcherServerListVersion = R.Version;

    if (UpdateDeviceSelectorDispatcherServerListCallback) {
        UpdateDeviceSelectorDispatcherServerListCallback(DeviceSelectorDispatcherServerListVersion, DeviceSelectorDispatcherSortedServerInfoList);
    }
    return true;
}

void xDeviceSelectorDispatcherServerListDownloader::PostDeviceSelectorDispatcherServerListRequest() {
    auto R    = xPP_DownloadDeviceSelectorDispatcherList();
    R.Version = DeviceSelectorDispatcherServerListVersion;
    PostMessage(Cmd_DownloadDeviceSelectorDispatcherServerList, 0, R);
}
