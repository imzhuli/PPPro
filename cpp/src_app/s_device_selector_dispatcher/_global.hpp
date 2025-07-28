#pragma once
#include "../lib_server_util/all.hpp"
#include "./client_side_service.hpp"
#include "./server_side_service.hpp"

extern xNetAddress BindAddressForClient;
extern xNetAddress ExportBindAddressForClient;
extern xNetAddress BindAddressForServer;
extern xNetAddress ExportBindAddressForServer;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;

extern xDSD_ClientSideService ClientSideService;
extern xDSD_ServerSideService ServerSideService;
