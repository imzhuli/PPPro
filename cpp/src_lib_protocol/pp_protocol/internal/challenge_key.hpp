#pragma once
#include <pp_common/_.hpp>

extern std::string        MakeChallengeKey(const xDeviceAddressPack & AddressPack);
extern xDeviceAddressPack ExtractChallengeKey(const std::string & Key);
