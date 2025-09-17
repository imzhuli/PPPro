#pragma once
#include "../lib_client/auth_client.hpp"
#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "./audit_account_server.hpp"
#include "./auth_cache_server.hpp"
#include "./client_base.hpp"
#include "./client_connection_manager.hpp"

#include <pp_common/_.hpp>

extern xNetAddress ConfigTcpBindAddress4;
extern xNetAddress ConfigTcpBindAddress6;
extern xNetAddress ConfigUdpBindAddress4;
extern xNetAddress ConfigUdpBindAddress6;

extern xNetAddress ConfigExportUdpServerAddress4;
extern xNetAddress ConfigExportUdpServerAddress6;

extern xNetAddress ConfigServerListDownloadAddress;

extern xAuditAccountServerListDownloader AuditAccountServerListDownloader;

extern xPA_ClientConnectionManager ClientConnectionManager;

extern xAuthClient AuthClient;
