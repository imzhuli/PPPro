#pragma once
#include "../lib_utils/all.hpp"

extern void InitAuthCacheLocalServer();
extern void CleanAuthCacheLocalServer();
extern void TickAuthCacheLocalServer(uint64_t NowMS);
extern bool PostAuthRequest(uint64_t RequestContextId, const std::string_view & AuthKey);
