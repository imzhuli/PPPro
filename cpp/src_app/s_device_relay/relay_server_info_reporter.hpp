#pragma once
#include "../lib_utils/all.hpp"

class xRelayInfoReporter final {

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
