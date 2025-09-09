#pragma once
#include "../lib_server_list/backend_server_list_downloader.hpp"
#include "../lib_utils/all.hpp"
#include "./auth_service.hpp"

#include <pp_common/_.hpp>

extern xNetAddress BindAddress;
extern xNetAddress ExportServerAddress;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;
extern xNetAddress ServerListDownloadAddress;

extern std::string BackendServerAppKey;
extern std::string BackendServerAppSecret;

extern uint64_t CacheNodeTimeoutMS;

////////////////////

extern xServerIdClient              ServerIdClient;
extern xRegisterServerClient        RegisterServerClient;
extern xBackendServerListDownloader BackendServerListDownloader;
extern xAC_AuthService              AuthService;
