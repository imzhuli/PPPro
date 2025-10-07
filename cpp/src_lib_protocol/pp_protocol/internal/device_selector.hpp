#pragma once
#include <pp_common/_.hpp>

struct xPP_RegisterDeviceSelector : public xBinaryMessage {

    void SerializeMembers() override {
        W(ServerInfo.RegionDetailLevel);
        W(ServerInfo.DevicePoolId);
        W(ServerInfo.StrategyFlags);
        //
    }

    void DeserializeMembers() override {
        R(ServerInfo.RegionDetailLevel);
        R(ServerInfo.DevicePoolId);
        R(ServerInfo.StrategyFlags);
    }

    xDeviceSelectorServerInfo ServerInfo;
};

struct xPP_AcquireDevice : public xBinaryMessage {

    void SerializeMembers() override {
        W(DevicePoolId);
        W(StrategyFlags);
        W(CountryId, StateId, CityId);
        W(RequireUdp, RequireRemoteDns);
        W(AccountView);
        W(OptionEx);
    }
    void DeserializeMembers() override {
        R(DevicePoolId);
        R(StrategyFlags);
        R(CountryId, StateId, CityId);
        R(RequireUdp, RequireRemoteDns);
        R(AccountView);
        R(OptionEx);
    }

    xDevicePoolId DevicePoolId;
    uint16_t      StrategyFlags;
    xCountryId    CountryId;
    xStateId      StateId;
    xCityId       CityId;
    bool          RequireUdp;
    bool          RequireRemoteDns;

    std::string_view AccountView;
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
