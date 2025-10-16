#pragma once
#include <pp_common/_.hpp>

struct xAD_BK_ReportUsageByAuditList : xBinaryMessage {

    static constexpr const size16_t MAX_USAGE_INFO_COUNT_PER_REPORT = 50;

    void SerializeMembers() override {
        W(LocalTimestampMS);
        W(AuditId);
        W(TotalSelectDeviceCountSinceLastPost);
        W(NoDeviceErrorCount);
        W(TotalTcpConnectionSinceLastPost);
        W(TotalTcpUploadSizeSinceLastPost);
        W(TotalTcpDownloadSizeSinceLastPost);
        W(TotalUdpChannelSinceLastPost);
        W(TotalUdpUploadSizeSinceLastPost);
        W(TotalUdpDownloadSizeSinceLastPost);
    }

    void DeserializeMembers() override {
        R(LocalTimestampMS);
        R(AuditId);
        R(TotalSelectDeviceCountSinceLastPost);
        R(NoDeviceErrorCount);
        R(TotalTcpConnectionSinceLastPost);
        R(TotalTcpUploadSizeSinceLastPost);
        R(TotalTcpDownloadSizeSinceLastPost);
        R(TotalUdpChannelSinceLastPost);
        R(TotalUdpUploadSizeSinceLastPost);
        R(TotalUdpDownloadSizeSinceLastPost);
    }

    uint64_t LocalTimestampMS;
    xAuditId AuditId;
    uint64_t TotalSelectDeviceCountSinceLastPost;  // 变更设备选择的次数(首次也计), 不包含涉及第三方的. 有可能在统计周期中没有变更选择, 这种情况是0.
    uint64_t NoDeviceErrorCount;
    uint64_t TotalTcpConnectionSinceLastPost;
    uint64_t TotalTcpUploadSizeSinceLastPost;
    uint64_t TotalTcpDownloadSizeSinceLastPost;
    uint64_t TotalUdpChannelSinceLastPost;
    uint64_t TotalUdpUploadSizeSinceLastPost;
    uint64_t TotalUdpDownloadSizeSinceLastPost;
};
