#pragma once
#include "../lib_server_util/all.hpp"

class xRelayInfoReporter {

public:
    bool Init(xIoContext * ICP);
    void Clean();
    void Tick(uint64_t NowMS);

    void UpdateServerAddress(const xNetAddress & Address);
    void UpdateLocalRelayServerInfo(const xRelayServerInfoBase & Info);

private:
    xRelayServerInfoBase LocalRelayServerInfo;
    xClientWrapper       Client;
};
