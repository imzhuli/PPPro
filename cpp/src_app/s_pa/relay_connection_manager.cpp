#include "./relay_connection_manager.hpp"

#include "../lib_server_list/relay_info_observer.hpp"
#include "./_global.hpp"

static auto RIO       = xRelayInfoObserver();
static auto RelayPool = xClientPoolWrapper();

void InitRelayConnectionManager() {
    RuntimeAssert(RIO.Init(ServiceIoContext, ConfigServerListDownloadAddress));
}

void CleanRelayConnectionManager() {
    RIO.Clean();
}

void TickRelayConnectionManager(uint64_t NowMS) {
    RIO.Tick(NowMS);
}
