#pragma once
#include "../lib_server_util/all.hpp"

class xRID_RegisterServerService {
public:
    bool Init();
    void Clean();
    void Tick(uint64_t NowMS);

private:
    xServerIdClient       ServerIdClient;
    xRegisterServerClient RegisterClient;
};
