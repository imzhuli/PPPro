#pragma once
#include <pp_common/_.hpp>

extern std::string          MakeChallengeKey(const xDeviceChallengePack & AddressPack);
extern xDeviceChallengePack ExtractChallengeKey(const std::string & Key);
