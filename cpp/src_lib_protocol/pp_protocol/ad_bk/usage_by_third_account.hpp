#pragma once
#include "../base.hpp"

struct xThirdAccountUsage {
    std::string ThirdAccountSourceName;  // 第三方账号名,
    uint64_t    TotalTcpConnectionSinceLastPost;
    uint32_t    TotalUdpChannelSinceLastPost;
    uint64_t    TotalUploadSizeSinceLastPost;
    uint64_t    TotalDownloadSizeSinceLastPost;
};

struct xAD_BK_ReportUsageByThirdAccountList : xBinaryMessage {

    static constexpr const size16_t MAX_USAGE_INFO_COUNT_PER_REPORT = 20;

    void SerializeMembers() override {
        assert(UsageList.size() < MAX_USAGE_INFO_COUNT_PER_REPORT);
        W(LocalTimestampMS);
        W(XR(uint32_t(UsageList.size())));
        for (auto & U : UsageList) {
            W(U.ThirdAccountSourceName);
            W(U.TotalTcpConnectionSinceLastPost);
            W(U.TotalUdpChannelSinceLastPost);
            W(U.TotalUploadSizeSinceLastPost);
            W(U.TotalDownloadSizeSinceLastPost);
        }
    }

    void DeserializeMembers() override {
        R(LocalTimestampMS);
        size32_t Count = 0;
        R(Count);
        if (Count > MAX_USAGE_INFO_COUNT_PER_REPORT) {
            xBinaryMessageReader::SetError();
            return;
        }
        UsageList.resize(Count);
        for (auto & U : UsageList) {
            R(U.ThirdAccountSourceName);
            R(U.TotalTcpConnectionSinceLastPost);
            R(U.TotalUdpChannelSinceLastPost);
            R(U.TotalUploadSizeSinceLastPost);
            R(U.TotalDownloadSizeSinceLastPost);
        }
    }

    uint64_t                        LocalTimestampMS;
    std::vector<xThirdAccountUsage> UsageList;
};
