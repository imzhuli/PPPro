#pragma once
#include <pp_common/_.hpp>

struct xPP_DeviceInfoUpdate : public xBinaryMessage {  // from proxy_access to relay server
public:
    void SerializeMembers() override {

        W(Version);
        W(ChannelId);
        W(RelayServerRuntimeId);
        W(RelayServerSideDeviceId);
        W(DeviceUuid);

        W(Tcp4Address, Udp4Address, Tcp6Address, Udp6Address);
        W(CountryId, StateId, CityId);
        W(IsOffline, SpeedLimitEnabled);
    }
    void DeserializeMembers() override {
        R(Version);
        R(ChannelId);
        R(RelayServerRuntimeId);
        R(RelayServerSideDeviceId);
        R(DeviceUuid);

        R(Tcp4Address, Udp4Address, Tcp6Address, Udp6Address);
        R(CountryId, StateId, CityId);
        R(IsOffline, SpeedLimitEnabled);
    }

    uint32_t Version;
    uint32_t ChannelId;
    uint64_t RelayServerRuntimeId;
    uint64_t RelayServerSideDeviceId;

    std::string DeviceUuid;
    xNetAddress Tcp4Address;
    xNetAddress Udp4Address;
    xNetAddress Tcp6Address;
    xNetAddress Udp6Address;

    xContinentId ContinentId;
    xCountryId   CountryId;
    xStateId     StateId;
    xCityId      CityId;

    bool IsOffline;  // 下线时会设置这个标志位. 但考虑到服务器的变更, 不是所有设备都有上/下线标志, 后台服务器应当将30分钟未发统计的设备设为下线.
    bool SpeedLimitEnabled;

    //
};
