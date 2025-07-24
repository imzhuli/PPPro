#pragma once
#include "../lib_server_util/all.hpp"
#include "./auth_service.hpp"
#include "./backend_server_list_downloader.hpp"

#include <pp_common/base.hpp>

extern xNetAddress BindAddress;
extern xNetAddress ExportServerAddress;

extern xNetAddress ServerIdCenterAddress;
extern xNetAddress ServerListRegisterAddress;
extern xNetAddress ServerListDownloadAddress;

extern std::string BackendServerAppKey;
extern std::string BackendServerAppSecret;

extern uint64_t CacheNodeTimeoutMS;
