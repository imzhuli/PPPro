#pragma once
#include <pp_common/_.hpp>

struct xPP_RegisterDeviceSelector : public xBinaryMessage {

    void SerializeMembers() override {
        W(ServerInfo.RegionDetailLevel);
        W(ServerInfo.PoolId);
        W(ServerInfo.StrategyFlags);
        //
    }

    void DeserializeMembers() override {
        R(ServerInfo.RegionDetailLevel);
        R(ServerInfo.PoolId);
        R(ServerInfo.StrategyFlags);
    }

    xDeviceSelectorServerInfo ServerInfo;
};

struct xPP_AcquireDevice : public xBinaryMessage {

    void SerializeMembers() override {
        W(StrategyFlags);
        W(CountryId, StateId, CityId);
        W(RequireIpv6, RequireUdp, RequireRemoteDns);
        W(OptionEx);
    }
    void DeserializeMembers() override {
        R(StrategyFlags);
        R(CountryId, StateId, CityId);
        R(RequireIpv6, RequireUdp, RequireRemoteDns);
        R(OptionEx);
    }

    uint16_t   StrategyFlags;
    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
    bool       RequireIpv6;
    bool       RequireUdp;
    bool       RequireRemoteDns;

    std::string_view OptionEx;
    //
};

struct xPP_AcquireDeviceResp : public xBinaryMessage {

public:
    void SerializeMembers() override { W(DeviceRelayServerRuntimeId, DeviceRelaySideId); }
    void DeserializeMembers() override { R(DeviceRelayServerRuntimeId, DeviceRelaySideId); }

    uint64_t DeviceRelayServerRuntimeId;
    uint64_t DeviceRelaySideId;
    //
};
