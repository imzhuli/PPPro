#pragma once
#include "../base.hpp"

struct xPP_DeviceInfoUpdate : public xBinaryMessage {  // from proxy_access to relay server
public:
    void SerializeMembers() override {
        W(DeviceUuid, Version, RelayServerRuntimeId, RelaySideDeviceKey, PrimaryIpv4Address, PrimaryIpv6Address);
        W(CountryId, StateId, CityId);
        W(IsOffline, SupportUdpChannel, SupportDnsRequests, SpeedLimitEnabled);
    }
    void DeserializeMembers() override {
        R(DeviceUuid, Version, RelayServerRuntimeId, RelaySideDeviceKey, PrimaryIpv4Address, PrimaryIpv6Address);
        R(CountryId, StateId, CityId);
        R(IsOffline, SupportUdpChannel, SupportDnsRequests, SpeedLimitEnabled);
    }

    std::string DeviceUuid;
    uint32_t    Version;
    uint64_t    RelayServerRuntimeId;
    uint64_t    RelaySideDeviceKey;
    xNetAddress PrimaryIpv4Address;
    xNetAddress PrimaryIpv6Address;

    xContinentId ContinentId;
    xCountryId   CountryId;
    xStateId     StateId;
    xCityId      CityId;

    bool IsOffline;  // 下线时会设置这个标志位. 但考虑到服务器的变更, 不是所有设备都有上/下线标志, 后台服务器应当将30分钟未发统计的设备设为下线.
    bool SupportUdpChannel;
    bool SupportDnsRequests;
    bool SpeedLimitEnabled;

    //
};
