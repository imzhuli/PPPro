#pragma once
#include "./auth_cache_server.hpp"
#include "./client_base.hpp"
#include "./client_connection_manager.hpp"

#include <pp_common/base.hpp>

extern xNetAddress ConfigTcpBindAddress;
extern xNetAddress ConfigServerListDownloadAddress;

extern xPA_AuthCacheServerListDownloader AuthCacheServerListDownloader;
extern xPA_ClientConnectionManager       ClientConnectionManager;
extern xPA_AuthCacheLocalServer          AuthCacheLocalServer;
