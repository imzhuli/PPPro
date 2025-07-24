#pragma once
#include <pp_common/_.hpp>
#include <pp_common/region.hpp>

class xRD_DeviceConnection;

struct xDevice : xListNode {
    static constexpr uint32_t FLAG_INIT   = 0x00;
    static constexpr uint32_t FLAG_BOUND  = 0x01;
    static constexpr uint32_t FLAG_DELETE = 0x80;

    xIndexId               DeviceRuntimeId     = 0;
    uint32_t               Flags               = FLAG_INIT;
    xRD_DeviceConnection * CtrlConnection      = 0;
    xRD_DeviceConnection * DataConnection      = 0;
    std::string            DeviceLocalIdString = {};
    xNetAddress            PrimaryIpv4         = {};
    xNetAddress            PrimaryIpv6         = {};
    xGeoInfo               GeoInfo             = {};
    size_t                 ReportCounter       = 0;

    void MarkDelete() { Flags |= FLAG_DELETE; }
    bool IsBeingDeleted() const { return Flags & FLAG_DELETE; }
};

class xDeviceManager {

public:
    bool Init(size_t MaxDevice);
    void Clean();
    void Tick(uint64_t NowMS);

    xDevice * GetDeviceById(uint64_t DeviceId) {
        auto Device = DevicePool.CheckAndGet(DeviceId);
        if (!Device) {
            return nullptr;
        }
        if (Device->IsBeingDeleted()) {
            X_DEBUG_PRINTF("device is being deleted");
            return nullptr;
        }
        return Device;
    }

public:
    auto NewDevice() -> xDevice * { return CreateDevice(); }
    void ReleaseDevice(xDevice * DC) { DeferDestroyDevice(DC); }
    void ReportDeviceOnState(uint64_t DeviceId) {
        auto PD = DevicePool.CheckAndGet(DeviceId);
        if (!PD) {
            return;
        }
        // if (!PD || (PD->ReportCounter++) % 5) {
        //     return;
        // }
        ReportDeviceOnState(PD);
    }
    void ReportDeviceOnState(xDevice * PD) { ReportDeviceState(PD, false); }
    void ReportDeviceOfflineState(uint64_t DeviceId) {
        auto PD = DevicePool.CheckAndGet(DeviceId);
        if (!PD) {
            return;
        }
        ReportDeviceOfflineState(PD);
    }
    void ReportDeviceOfflineState(xDevice * PD) { ReportDeviceState(PD, true); }

protected:
    auto CreateDevice() -> xDevice *;
    void DeferDestroyDevice(xDevice * DC) {
        DC->MarkDelete();
        DeviceKillList.GrabTail(*DC);
    }
    void DestroyDevice(xDevice * DC);
    void ReportDeviceState(xDevice * Device, bool Offline);

private:
    xTicker                  Ticker;
    xIndexedStorage<xDevice> DevicePool;
    xList<xDevice>           DeviceKillList;
    //
};
