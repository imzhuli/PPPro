#pragma once
#include <pp_common/base.hpp>
#include <unordered_map>

static constexpr const uint64_t DEVICE_KEEPALIVE_TIMEOUT_MS = 180'000;  // normally device is removed by request from dispatcher
static constexpr const size_t   DEVICE_INFO_RESIST_COUNTER  = 2;        // normally device is removed by request from dispatcher

struct xDR_TimeoutNode : xListNode {
    uint64_t TimestampMS;
};
struct xDR_CountryNode : xListNode {};
struct xDR_StateNode : xListNode {};
struct xDR_CityNode : xListNode {};

struct xDR_DeviceInfoBase {
    uint64_t    ReleayServerRuntimeId;
    uint64_t    RelaySideDeviceId;
    std::string DeviceId;

    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
};

struct xDS_DeviceContext
    : xDR_TimeoutNode
    , xDR_CountryNode
    , xDR_StateNode
    , xDR_CityNode {
    xDR_DeviceInfoBase InfoBase;
    size_t             ResisterCounter = DEVICE_INFO_RESIST_COUNTER;
};

class xDS_DeviceContextManager {
public:
    bool Init();
    void Clean();
    void Tick(uint64_t NowMS);

    void UpdateDevice(const xDR_DeviceInfoBase & InfoBase);
    void RemoveDevice(xDS_DeviceContext * Device);
    void RemoveDeviceById(const std::string & DeviceId);

    const xDS_DeviceContext * SelectDeviceByCountryId(xCountryId Id);
    const xDS_DeviceContext * SelectDeviceByStateId(xStateId Id);
    const xDS_DeviceContext * SelectDeviceByCityId(xCityId Id);

protected:
    void KeepAlive(xDS_DeviceContext * Device);

private:
    xTicker                                              Ticker;
    xList<xDR_TimeoutNode>                               TimeoutDeviceList;
    std::unordered_map<uint32_t, xList<xDR_CountryNode>> CountryDeviceList;
    std::unordered_map<uint32_t, xList<xDR_StateNode>>   StateDeviceList;
    std::unordered_map<uint32_t, xList<xDR_CityNode>>    CityDeviceList;
    std::unordered_map<std::string, xDS_DeviceContext *> DeviceMap;
};
