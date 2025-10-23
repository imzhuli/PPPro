#include "./audit_account_server.hpp"

#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_utils/all.hpp"
#include "./_global.hpp"

static xAuditAccountServerListDownloader AASLDownloader;
static constexpr const uint64_t          AuditAccountTimeoutMS = 5 * 60'000;

struct xAuditAccountNode : xListNode {
    uint64_t LastReportTimestampMS = 0;
    xAuditId AuditId               = 0;
    uint64_t NoDeviceErrorCount;
    uint64_t TotalTcpConnectionSinceLastPost;
    uint64_t TotalTcpUploadSizeSinceLastPost;
    uint64_t TotalTcpDownloadSizeSinceLastPost;
    uint64_t TotalUdpChannelSinceLastPost;
    uint64_t TotalUdpUploadSizeSinceLastPost;
    uint64_t TotalUdpDownloadSizeSinceLastPost;
};

static std::unordered_map<uint64_t /* AuditId*/, xAuditAccountNode> LocalAccountAuditMap;
static xel::xList<xAuditAccountNode>                                LocalAuditTimeoutList;
static xClientPoolWrapper                                           AuditClient;

[[maybe_unused]]
static bool NeedAuditReport(const xAuditAccountNode * NodePtr) {
    auto Now = ServiceTicker();
    if (Now - NodePtr->LastReportTimestampMS >= 5 * 60'000) {
        return true;
    }
    if (NodePtr->TotalTcpUploadSizeSinceLastPost > 40'000) {
        return true;
    }
    if (NodePtr->TotalTcpUploadSizeSinceLastPost > 50'000) {
        return true;
    }
    if (NodePtr->TotalUdpUploadSizeSinceLastPost > 40'000) {
        return true;
    }
    if (NodePtr->TotalUdpDownloadSizeSinceLastPost > 50'000) {
        return true;
    }
    return false;
}

void InitAuditAccountService() {
    RuntimeAssert(AASLDownloader.Init(ServiceIoContext, ConfigServerListDownloadAddress));
}

void CleanAuditAccountService() {
    AASLDownloader.Clean();
    Renew(LocalAccountAuditMap);
}

static void ReportAuditAccount(const xAuditAccountNode * PAA) {
    auto R = PAA->NoDeviceErrorCount || PAA->TotalTcpConnectionSinceLastPost || PAA->TotalTcpUploadSizeSinceLastPost || PAA->TotalTcpDownloadSizeSinceLastPost ||
        PAA->TotalUdpChannelSinceLastPost || PAA->TotalUdpUploadSizeSinceLastPost || PAA->TotalUdpDownloadSizeSinceLastPost;
    if (!R) {
        return;
    }
}

void TickAuditAccountService(uint64_t NowMS) {
    //
    auto KillTimepointMS = NowMS - AuditAccountTimeoutMS;
    auto Pred            = [KillTimepointMS](const xAuditAccountNode & N) { return N.LastReportTimestampMS <= KillTimepointMS; };
    while (auto PN = LocalAuditTimeoutList.PopHead(Pred)) {
        ReportAuditAccount(PN);
    }
}
