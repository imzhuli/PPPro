#pragma once
#include "../base.hpp"

// !!! 总则说明
// 在服务器发生变更时, 有可能同一UUID的服务器在同一时段的被多次上报,
// 这是因为服务器的上报数据可能跑到不同的缓存服务器上

// 逻辑上正确的处理如下
// 对于绝大多数的数据, 此时后端应将其相加.
// 对于有"当前/最新"这种有明确时刻的统计参数, 指标值 (如当前设备数量, 只须以任一次为准即可)

// 实际上可以简化此逻辑: 因为这种情况基本只能发生在服务器变动的时候.
// 直接认为是先后两次独立的上报. 尽管在细粒度的图(如秒)可能出现峰值. 但在粗粒度(如10分钟)上, 可以保持数据的正确.

// !!! 关于ServerUuid 和 ServerIndex
// 原则上ServerUuid应写在配置文件中, 或启动时自动生成.
// 以后的版本应当 !!!不会改为如果配置没有则自动生成后写回配置文件!!!. 主要是因为部分服务不适合全自动部署(如第三方/静态服务器)
// 不同类型的服务器, ServerIndex是可能重叠的.  这个主要是考虑到服务器列表的下载和更新的开销(比如PA下载RL, 连接池重新生成时, 能减少合并时间, 并避开重复建立连接)
// 后台统计服务, 目前可以忽略这个值. 这个字段主要是保留给以后监控服务器异常重启使用.(比如同一ServerIndex的服务发生了Uuid的变更, 可以发一条报警信息)
//

struct xAD_BK_StaticRelayServerInfo {
    // 后继版本会限制每一个静态服务进程所绑定的IP/端口数量, 暂定50
    std::string              ServerUuid;
    uint16_t                 ServerIndex;
    uint16_t                 BindAddressCount;
    std::vector<xNetAddress> BindAddressList;
};

struct xAD_BK_ThirdRelayServerInfo {
    // 待定
};

struct xAD_BK_DeivceRelayServerInfo {

    static constexpr const xServerRouteId INVALID_ROUTE_ID = (uint16_t)(-1);

    std::string    ServerUuid;
    uint64_t       ServerRuntimeId;                   // 这是运行时由配置中心给的值, 在同一时刻不会重复,
    xServerRouteId ServerRouteId = INVALID_ROUTE_ID;  // 为减少服务器间延迟而给服务器硬写的区块ID.
    xNetAddress    ExportCtrlAddress;
    xNetAddress    ExportDataAddress;
    xNetAddress    ExportPAAddress;

    uint32_t LatestDeviceCount;                  // 最新时刻的设备数量
    uint32_t NewOnlineDevcieSinceLastPost;       // 新连线的设备数量 (不去重)
    uint32_t NewOfflineDeviceSinceLastPost;      // 新掉线的设备数量 (不去重)
    uint64_t UploadToDeviceSinceLastPost;        // unit byte
    uint64_t DownloadFromDeviceSinceLastReport;  // unit byte

    //
};

struct xAD_BK_ReportDeviceRelayServerInfoList : xBinaryMessage {

    static constexpr const size16_t MAX_SERVER_INFO_COUNT_PER_REPORT = 25;

    void SerializeMembers() override {
        assert(ServerInfoList.size() < MAX_SERVER_INFO_COUNT_PER_REPORT);
        for (auto & S : ServerInfoList) {
            W(S.ServerUuid);
            W(S.ServerRuntimeId);
            W(S.ServerRouteId);
            W(S.ExportCtrlAddress);
            W(S.ExportDataAddress);
            W(S.ExportPAAddress);

            W(S.LatestDeviceCount);
            W(S.NewOnlineDevcieSinceLastPost);
            W(S.NewOfflineDeviceSinceLastPost);
            W(S.UploadToDeviceSinceLastPost);
            W(S.DownloadFromDeviceSinceLastReport);
        }
    };

    void DeserializeMembers() override {
        size16_t Count = 0;
        R(Count);
        if (Count > MAX_SERVER_INFO_COUNT_PER_REPORT) {
            xBinaryMessageReader::SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & S : ServerInfoList) {
            R(S.ServerUuid);
            R(S.ServerRuntimeId);
            R(S.ServerRouteId);
            R(S.ExportCtrlAddress);
            R(S.ExportDataAddress);
            R(S.ExportPAAddress);

            R(S.LatestDeviceCount);
            R(S.NewOnlineDevcieSinceLastPost);
            R(S.NewOfflineDeviceSinceLastPost);
            R(S.UploadToDeviceSinceLastPost);
            R(S.DownloadFromDeviceSinceLastReport);
        }
    };

    std::vector<xAD_BK_DeivceRelayServerInfo> ServerInfoList;
};
