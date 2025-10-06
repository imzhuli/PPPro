#pragma once
#include "../lib_utils/all.hpp"
#include "./observer.hpp"
#include "./producer.hpp"

#include <pp_common/_.hpp>

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;

extern xNetAddress ProducerAddress4;
extern xNetAddress ObserverAddress4;
extern xNetAddress ExportProducerAddress4;
extern xNetAddress ExportObserverAddress4;

extern xTcpService ProducerService4;
extern xTcpService ObserverService4;

struct xRID_LocalAudit {
    size_t TotalRelayConnections          = 0;
    size_t TotalAvailableRelayConnections = 0;
    size_t TotalRegisteredConnections     = 0;
    size_t TotalClosedConnections         = 0;

    size_t CurrentObserverCount = 0;
};

class xRID_LocalAuditLogger {
public:
    static constexpr const uint64_t LocalAuditTimeoutMS = 60'000;

    void Tick(uint64_t NowMS);

private:
    uint64_t LastLogTimestampMS = xel::GetTimestampMS();
};

extern xRID_LocalAudit       LocalAudit;
extern xRID_LocalAuditLogger LocalAuditLogger;
