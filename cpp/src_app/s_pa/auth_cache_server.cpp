#include "./auth_cache_server.hpp"

#include "../lib_client/auth_client.hpp"
#include "../lib_utils/all.hpp"
#include "./_global.hpp"

static xAuthClient AuthClient;

void InitAuthCacheLocalServer() {
    AuthClient.OnAuthCacheResultCallback = OnPAC_AuthResult;
    RuntimeAssert(AuthClient.Init(ServiceIoContext, ConfigServerListDownloadAddress));
}

void CleanAuthCacheLocalServer() {
    AuthClient.Clean();
}

void TickAuthCacheLocalServer(uint64_t NowMS) {
    AuthClient.Tick(NowMS);
}

bool PostAuthRequest(uint64_t RequestContextId, const std::string_view & AuthKey) {
    return AuthClient.Request(RequestContextId, AuthKey);
}
