#include "../lib_utils/all.hpp"
#include "./_global.hpp"
#include "./device_context_manager.hpp"

#include <pp_protocol/command.hpp>

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Require(BindAddress, "BindAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    X_GUARD(DeviceSelectorService, ServiceIoContext);
    X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(DSDDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(DeviceObserver, ServiceIoContext);

    DSRDownloader.SetOnUpdateDeviceStateRelayServerListCallback([](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & DSRInfo) {
        auto OSL = std::vector<xNetAddress>();
        for (auto & S : DSRInfo) {
            OSL.push_back(S.ObserverAddress);
        }
        DeviceObserver.UpdateServerList(OSL);
    });
    DSDDownloader.SetOnUpdateDeviceSelectorDispatcherServerListCallback([](uint32_t Version, const std::vector<xDeviceSelectorDispatcherInfo> & ServerList) {
        DeviceSelectorService.UpdateDispatcherList(ServerList);
    });
    DeviceObserver.SetOnPacketCallback([](const xMessageChannel & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) -> bool {
        switch (CommandId) {
            case Cmd_DSR_DS_DeviceUpdate: {
                auto PP = xPP_DeviceInfoUpdate();
                if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
                    DEBUG_LOG("Invalid device info");
                    return true;
                }

                if (!PP.IsOffline) {
                    auto LocalDevInfo                    = xDR_DeviceInfoBase{};
                    LocalDevInfo.DeviceId                = PP.DeviceUuid;
                    LocalDevInfo.ReleayServerRuntimeId   = PP.RelayServerRuntimeId;
                    LocalDevInfo.RelayServerSideDeviceId = PP.RelayServerSideDeviceId;

                    LocalDevInfo.CountryId = PP.CountryId;
                    LocalDevInfo.StateId   = PP.StateId;
                    LocalDevInfo.CityId    = PP.CityId;

                    DeviceContextManager.UpdateDevice(LocalDevInfo);
                } else {
                    DeviceContextManager.RemoveDeviceById(PP.DeviceUuid);
                }
                return true;
            }

            default:
                DEBUG_LOG("Unknown CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
                break;
        }
        return true;
    });

    auto AuditTimestampMS = ServiceTicker();
    while (true) {
        ServiceUpdateOnce(DeviceSelectorService, DSRDownloader, DSDDownloader, DeviceObserver, DeviceContextManager);

        LocalAudit.DurationMS = ServiceTicker() - AuditTimestampMS;
        if (LocalAudit.DurationMS >= 60'000) {
            AuditLogger->I("%s", LocalAudit.ToString().c_str());
            LocalAudit.ResetPeriodCount();
            AuditTimestampMS = ServiceTicker();
        }
    }
    return 0;
}
