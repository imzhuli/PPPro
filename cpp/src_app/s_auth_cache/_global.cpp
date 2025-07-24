#include "./_global.hpp"

xNetAddress BindAddress;
xNetAddress ExportServerAddress;

xNetAddress ServerIdCenterAddress;
xNetAddress ServerListRegisterAddress;
xNetAddress ServerListDownloadAddress;

std::string BackendServerAppKey;
std::string BackendServerAppSecret;

uint64_t CacheNodeTimeoutMS = 10 * 60'000;
