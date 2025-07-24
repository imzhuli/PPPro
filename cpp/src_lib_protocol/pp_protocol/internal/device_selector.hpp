#include "../base.hpp"

struct xPP_AcquireDevice : public xBinaryMessage {

    void SerializeMembers() override {
        W(CountryId, StateId, CityId, RequireIpv6, RequireUdp, RequireRemoteDns);
    }
    void DeserializeMembers() override {
        R(CountryId, StateId, CityId, RequireIpv6, RequireUdp, RequireRemoteDns);
    }

    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
    bool       RequireIpv6;
    bool       RequireUdp;
    bool       RequireRemoteDns;
    //
};

struct xPP_AcquireDeviceResp : public xBinaryMessage {

public:
    void SerializeMembers() override {
        W(DeviceRelayServerRuntimeId, DeviceRelaySideId);
    }
    void DeserializeMembers() override {
        R(DeviceRelayServerRuntimeId, DeviceRelaySideId);
    }

    uint64_t DeviceRelayServerRuntimeId;
    uint64_t DeviceRelaySideId;
    //
};
