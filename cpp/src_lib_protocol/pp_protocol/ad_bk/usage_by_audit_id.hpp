#pragma once
#include <pp_common/_.hpp>

struct xAD_BK_ReportUsageByAuditAccount : xBinaryMessage {

    static constexpr const size16_t MAX_USAGE_INFO_COUNT_PER_REPORT = 50;

    void SerializeMembers() override {
        W(LocalTimestampMS);
        W(AuditInfo.AuditId);
        W(AuditInfo.TotalTcpCount);
        W(AuditInfo.TotalTcpUploadSize);
        W(AuditInfo.TotalTcpDownloadSize);
        W(AuditInfo.TotalUdpCount);
        W(AuditInfo.TotalUdpUploadSize);
        W(AuditInfo.TotalUdpDownloadSize);
    }

    void DeserializeMembers() override {
        R(LocalTimestampMS);
        R(AuditInfo.AuditId);
        R(AuditInfo.TotalTcpCount);
        R(AuditInfo.TotalTcpUploadSize);
        R(AuditInfo.TotalTcpDownloadSize);
        R(AuditInfo.TotalUdpCount);
        R(AuditInfo.TotalUdpUploadSize);
        R(AuditInfo.TotalUdpDownloadSize);
    }

    uint64_t          LocalTimestampMS;
    xAuditAccountInfo AuditInfo;
};
