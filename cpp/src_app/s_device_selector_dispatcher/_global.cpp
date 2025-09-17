#include "./_global.hpp"

xNetAddress BindAddressForClient;
xNetAddress ExportBindAddressForClient;
xNetAddress BindAddressForServer;
xNetAddress ExportBindAddressForServer;

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListRegisterAddress;

xTcpService ClientSideService;
xTcpService ServerSideService;

xServiceRequestContextPool RequestContextPool;

xDSD_ServiceProviderManager ServiceProviderManager;

xDSD_LocalAudit LocalAudit;

static uint64_t LastOutputLocalAuditTimestampMS = 0;

void OutputLocalAudit() {
    auto NowMS = ServiceTicker();
    if (NowMS - LastOutputLocalAuditTimestampMS < 5 * 60'000) {
        return;
    }
    LastOutputLocalAuditTimestampMS = NowMS;

    auto OS = std::ostringstream();
    OS << "DuplicateRegistation: " << Steal(LocalAudit.TotalDuplicateRegistation) << ' ';
    OS << "AddServerInfoError: " << Steal(LocalAudit.TotalAddServerInfoError) << ' ';

    OS << "CurrentServiceProviderCount: " << LocalAudit.CurrentServiceProviderCount << ' ';
    AuditLogger->I(OS.str().c_str());
}
