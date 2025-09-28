#include "./audit_account_server.hpp"

#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_utils/all.hpp"
#include "./_global.hpp"

static xAuditAccountServerListDownloader AASLDownloader;

void InitAuditAccountService() {
    RuntimeAssert(AASLDownloader.Init(ServiceIoContext, ConfigServerListDownloadAddress));
}

void CleanAuditAccountService() {
    AASLDownloader.Clean();
}

void TickAuditAccountService(uint64_t NowMS) {
    //
}
