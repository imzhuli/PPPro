#pragma once
#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "./audit_account_server.hpp"
#include "./auth_cache_server.hpp"
#include "./client_base.hpp"
#include "./client_connection_manager.hpp"

#include <pp_common/_.hpp>

extern xNetAddress ConfigTcpBindAddress;
extern xNetAddress ConfigServerListDownloadAddress;

extern xAuditAccountServerListDownloader AuditAccountServerListDownloader;
extern xAuthCacheServerListDownloader    AuthCacheServerListDownloader;

extern xPA_ClientConnectionManager ClientConnectionManager;
