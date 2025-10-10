#include "./device_reporter.hpp"

#include "./_global.hpp"

#include <pp_protocol/ad_bk/device_info.hpp>

#ifndef NDEBUG
static constexpr const uint64_t MIN_DEVICE_KEEPALIVE_TIMEOUT_MS = 0;
#else
static constexpr const uint64_t MIN_DEVICE_KEEPALIVE_TIMEOUT_MS = 2 * 60'000 + 15'000;
#endif

static xPP_DeviceInfoUpdate PrepareReport(const xDR_DeviceContext * PDC) {
    auto DI       = xPP_DeviceInfoUpdate();
    DI.Version    = PDC->Version;
    DI.ChannelId  = PDC->ChannelId;
    DI.DeviceUuid = PDC->Uuid;

    DI.RelayServerRuntimeId    = ServerIdClient.GetLocalServerId();
    DI.RelayServerSideDeviceId = PDC->Id;

    DI.Tcp4Address = PDC->Tcp4Address;
    DI.Udp4Address = PDC->Udp4Address;
    DI.Tcp6Address = PDC->Tcp6Address;
    DI.Udp6Address = PDC->Udp6Address;

    DI.CountryId = PDC->GeoInfo.CountryId;
    DI.StateId   = PDC->GeoInfo.StateId;
    DI.CityId    = PDC->GeoInfo.CityId;

    return DI;
}

void ReportKeepAliveDevice(const xDR_DeviceContext * PDC) {
    if (ServiceTicker() - PDC->StartupTimestampMS < MIN_DEVICE_KEEPALIVE_TIMEOUT_MS) {
        return;
    }
    auto DI = PrepareReport(PDC);
    DeviceReporter.PostMessageByHash(DI.RelayServerRuntimeId, Cmd_DSR_DS_DeviceUpdate, 0, DI);
}

void ReportDeviceDrop(const xDR_DeviceContext * PDC) {
    auto DI      = PrepareReport(PDC);
    DI.IsOffline = true;
    DeviceReporter.PostMessageByHash(DI.RelayServerRuntimeId, Cmd_DSR_DS_DeviceUpdate, 0, DI);
}
