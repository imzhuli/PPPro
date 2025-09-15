#include "./device_reporter.hpp"

#include "./_global.hpp"

#include <pp_protocol/ad_bk/device_info.hpp>

// struct xAD_BK_DeviceInfo {

//     uint32_t    Version;
//     uint32_t    ChannelId;
//     std::string DeviceUuid;
//     uint64_t    RelayServerRuntimeId;
//     uint64_t    RelayServerSideDeviceId;
//     xNetAddress PrimaryIpv4Address;
//     xNetAddress PrimaryIpv6Address;

//     uint32_t CountryId;
//     uint32_t StateId;
//     uint32_t CityId;

//     bool IsOffline;  // 下线时会设置这个标志位. 但考虑到服务器的变更, 不是所有设备都有上/下线标志, 后台服务器应当将30分钟未发统计的设备设为下线.
//     bool SupportUdpChannel;
//     bool SupportDnsRequests;
//     bool SpeedLimitEnabled;

//     uint64_t TotalOnlineTimeMS;  // 从上线时算起, 假设中间出现了统计丢失 (如后台认为30分钟下线的设备, 但后来又收到此数值比较大,
//                                  // 可由后台简单处理, 比如理解成两个相同DeviceUuid平行时空的设备, 分别上线和下线, 因为实际业务中没必要处理此冲突)
//     uint32_t TotalNewConnectionsSinceLastPost;
//     uint32_t TotalClosedConnectionSinceLastPost;
//     uint32_t TotalNewUdpChannelSinceLastPost;
//     uint32_t TotalClosedUdpChannelSinceLastPost;
//     uint32_t TotalDnsRequestSinceLastPost;

//     uint64_t TotalUploadSizeSinceOnline;    // 指发向目标的数据
//     uint64_t TotalDownloadSizeSinceOnline;  // 下载数据

//     uint32_t CurrentConnectionCount;
//     uint32_t CurrentUdpChannelCount;

//     uint32_t MaxConnectionCount;
//     uint32_t MaxUdpChannelCount;
//     //
// };

static xPP_DeviceInfoUpdate PrepareReport(const xDR_DeviceContext * PDC) {
    auto DI       = xPP_DeviceInfoUpdate();
    DI.Version    = PDC->Version;
    DI.ChannelId  = PDC->ChannelId;
    DI.DeviceUuid = PDC->Uuid;

    DI.RelayServerRuntimeId    = ServerIdClient.GetLocalServerId();
    DI.RelayServerSideDeviceId = PDC->Id;

    return DI;
}

void ReportNewDevice(const xDR_DeviceContext * PDC) {
    auto DI = PrepareReport(PDC);
    DeviceReporter.PostMessageByHash(DI.RelayServerRuntimeId, Cmd_DSR_DS_DeviceUpdate, 0, DI);
}

void ReportKeepAliveDevice(const xDR_DeviceContext * PDC) {
    auto DI = PrepareReport(PDC);
    DeviceReporter.PostMessageByHash(DI.RelayServerRuntimeId, Cmd_DSR_DS_DeviceUpdate, 0, DI);
}

void ReportDeviceDrop(const xDR_DeviceContext * PDC) {
    auto DI      = PrepareReport(PDC);
    DI.IsOffline = true;
    DeviceReporter.PostMessageByHash(DI.RelayServerRuntimeId, Cmd_DSR_DS_DeviceUpdate, 0, DI);
}
