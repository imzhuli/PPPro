#pragma once
#include <pp_common/_.hpp>

struct xAD_BK_ReportUsageByThirdAccountList : xBinaryMessage {

    static constexpr const size16_t MAX_USAGE_INFO_COUNT_PER_REPORT = 20;

    void SerializeMembers() override {
        W(LocalTimestampMS);
        W(AuditId);
        W(ThirdAccountSourceName);
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
        R(ThirdAccountSourceName);
        R(TotalTcpConnectionSinceLastPost);
        R(TotalTcpUploadSizeSinceLastPost);
        R(TotalTcpDownloadSizeSinceLastPost);
        R(TotalUdpChannelSinceLastPost);
        R(TotalUdpUploadSizeSinceLastPost);
        R(TotalUdpDownloadSizeSinceLastPost);
    }

    uint64_t    LocalTimestampMS;
    uint64_t    AuditId;
    std::string ThirdAccountSourceName;  // 第三方账号名,
    uint64_t    TotalTcpConnectionSinceLastPost;
    uint64_t    TotalTcpUploadSizeSinceLastPost;
    uint64_t    TotalTcpDownloadSizeSinceLastPost;
    uint64_t    TotalUdpChannelSinceLastPost;
    uint64_t    TotalUdpUploadSizeSinceLastPost;
    uint64_t    TotalUdpDownloadSizeSinceLastPost;

    //
};
