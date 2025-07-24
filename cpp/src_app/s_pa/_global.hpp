#pragma once
#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "./auth_cache_server.hpp"
#include "./client_base.hpp"
#include "./client_connection_manager.hpp"

#include <pp_common/base.hpp>

extern xNetAddress ConfigTcpBindAddress;
extern xNetAddress ConfigServerListDownloadAddress;

extern xAuthCacheServerListDownloader AuthCacheServerListDownloader;
extern xPA_ClientConnectionManager    ClientConnectionManager;
extern xPA_AuthCacheLocalServer       AuthCacheLocalServer;
