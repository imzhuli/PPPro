#include "./global.hpp"

#include "../lib_utils/all.hpp"

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListRegisterAddress;

xNetAddress ProducerAddress4;
xNetAddress ObserverAddress4;
xNetAddress ExportProducerAddress4;
xNetAddress ExportObserverAddress4;

xTcpService ProducerService4;
xTcpService ObserverService4;

xRID_LocalAudit       LocalAudit;
xRID_LocalAuditLogger LocalAuditLogger;

#define O(x) OS << #x << "=" << LocalAudit.x << endl
static std::string LocalAuditToString() {
    auto OS = std::ostringstream();
    O(TotalRelayConnections);
    O(TotalAvailableRelayConnections);
    O(TotalRegisteredConnections);
    O(TotalClosedConnections);

    O(CurrentObserverCount);
    return OS.str();
}
#undef O

void xRID_LocalAuditLogger::Tick(uint64_t NowMS) {
    if (NowMS - LastLogTimestampMS < LocalAuditTimeoutMS) {
        return;
    }
    LastLogTimestampMS = NowMS;
    AuditLogger->I("%s", LocalAuditToString().c_str());
}
