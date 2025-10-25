#pragma once
#include "../lib_client/auth_client.hpp"
#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "./auth_cache_server.hpp"
#include "./client_manager.hpp"
#include "./device_selector_manager.hpp"
#include "./relay_connection_manager.hpp"

#include <pp_common/_.hpp>

extern xNetAddress ConfigTcpBindAddress;
extern std::string ConfigExportTcpBindAddress;  // used for while list authentication
extern xNetAddress ConfigServerListDownloadAddress;
extern std::string ConfigUdpMapping;

extern std::vector<std::pair<xNetAddress, xNetAddress>> UdpMappingList;
