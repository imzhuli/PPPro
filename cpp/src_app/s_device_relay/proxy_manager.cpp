#include "./proxy_manager.hpp"

#include "./_global.hpp"

static xTcpService ProxyService4;

void InitProxyService() {
    RuntimeAssert(ProxyService4.Init(ServiceIoContext, ProxyAddress4, 20000));
}

void CleanProxyService() {
    ProxyService4.Clean();
}

void TickProxyService(uint64_t NowMS) {
    ProxyService4.Tick(NowMS);
}
