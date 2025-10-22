#pragma once
#include "../lib_client/auth_client.hpp"
#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "./audit_account_server.hpp"
#include "./auth_cache_server.hpp"
#include "./client_manager.hpp"
#include "./client_manager_udp.hpp"
#include "./device_selector_manager.hpp"
#include "./relay_connection_manager.hpp"

#include <pp_common/_.hpp>

extern xNetAddress ConfigTcpBindAddress;
extern xNetAddress ConfigExportUdpServerAddress;
extern xNetAddress ConfigServerListDownloadAddress;
extern std::string ConfigUdpMapping;