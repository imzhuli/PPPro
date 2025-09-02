#pragma once
#include <pp_common/_.hpp>

struct xAD_BK_UsageByAuditId {
    xAuditId AuditId;
    uint32_t TotalSelectDeviceCountSinceLastPost;  // 变更设备选择的次数(首次也计), 不包含涉及第三方的. 有可能在统计周期中没有变更选择, 这种情况是0.
    uint32_t TotalTcpConnectionSinceLastPost;
    uint32_t TotalUdpChannelSinceLastPost;
    uint64_t TotalUploadSizeSinceLastPost;
    uint64_t TotalDownloadSizeSinceLastPost;
    //
};

struct xAD_BK_ReportUsageByAuditList : xBinaryMessage {

    static constexpr const size16_t MAX_USAGE_INFO_COUNT_PER_REPORT = 50;

    void SerializeMembers() override {
        assert(AuditList.size() < MAX_USAGE_INFO_COUNT_PER_REPORT);
        W(LocalTimestampMS);
        W(XR(uint32_t(AuditList.size())));
        for (auto & U : AuditList) {
            W(U.AuditId);
            W(U.TotalSelectDeviceCountSinceLastPost);
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
        AuditList.resize(Count);
        for (auto & U : AuditList) {
            R(U.AuditId);
            R(U.TotalSelectDeviceCountSinceLastPost);
            R(U.TotalTcpConnectionSinceLastPost);
            R(U.TotalUdpChannelSinceLastPost);
            R(U.TotalUploadSizeSinceLastPost);
            R(U.TotalDownloadSizeSinceLastPost);
        }
    }

    uint64_t                           LocalTimestampMS;
    std::vector<xAD_BK_UsageByAuditId> AuditList;
};
