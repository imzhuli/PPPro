#pragma once
#include <pp_common/_.hpp>

struct xPP_RegisterDeviceSelector : public xBinaryMessage {

    void SerializeMembers() override {
        W(ServerInfo.RegionDetailLevel);
        W(ServerInfo.AllowRegionDowngrade);
        W(ServerInfo.HasAuditBinding);
        W(ServerInfo.PoolId);
        W(ServerInfo.StrategyId);
        //
    }

    void DeserializeMembers() override {
        R(ServerInfo.RegionDetailLevel);
        R(ServerInfo.AllowRegionDowngrade);
        R(ServerInfo.HasAuditBinding);
        R(ServerInfo.PoolId);
        R(ServerInfo.StrategyId);
    }

    xDeviceSelectorServerInfo ServerInfo;
};

struct xPP_AcquireDevice : public xBinaryMessage {

    void SerializeMembers() override {
        W(CountryId, StateId, CityId);
        W(RequireIpv6, RequireUdp, RequireRemoteDns);
        W(OptionEx);
    }
    void DeserializeMembers() override {
        R(CountryId, StateId, CityId);
        R(RequireIpv6, RequireUdp, RequireRemoteDns);
        R(OptionEx);
    }

    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
    bool       RequireIpv6;
    bool       RequireUdp;
    bool       RequireRemoteDns;

    std::string OptionEx;
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
