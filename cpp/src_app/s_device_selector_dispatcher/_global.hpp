#pragma once
#include "../lib_utils/all.hpp"
#include "./client_side_service.hpp"
#include "./server_side_service.hpp"
#include "./service_provider_manager.hpp"

#include <pp_protocol/command.hpp>

extern xNetAddress BindAddressForClient;
extern xNetAddress ExportBindAddressForClient;
extern xNetAddress BindAddressForServer;
extern xNetAddress ExportBindAddressForServer;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;

extern xTcpService ClientSideService;
extern xTcpService ServerSideService;

extern xServiceRequestContextPool RequestContextPool;

extern xDSD_ServiceProviderManager ServiceProviderManager;

struct xDSD_LocalAudit {
    uint64_t TotalDuplicateRegistation = 0;
    uint64_t TotalAddServerInfoError   = 0;

    uint64_t CurrentServiceProviderCount = 0;
};
extern xDSD_LocalAudit LocalAudit;

extern void OutputLocalAudit();
