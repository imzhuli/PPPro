#include "../base.hpp"

struct xPP_RegisterDeviceSelector : public xBinaryMessage {

    void SerializeMembers() override {
        W(ServerInfo.RegionDetailLevel);
        W(ServerInfo.AllowRegionDowngrade);
        W(ServerInfo.HasAuditBinding);
        W(XR(ServerInfo.PoolFlags.to_string()));
        //
    }
    void DeserializeMembers() override {
        auto PoolFlagsString = std::string();
        R(ServerInfo.RegionDetailLevel);
        R(ServerInfo.AllowRegionDowngrade);
        R(ServerInfo.HasAuditBinding);
        R(PoolFlagsString);
        try {
            xel::Reset(ServerInfo.PoolFlags, PoolFlagsString);
        } catch (...) {
            GetReader()->SetError();
        }
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
