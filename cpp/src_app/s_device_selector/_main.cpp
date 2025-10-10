#include "../lib_utils/all.hpp"
#include "./_global.hpp"
#include "./device_context_manager.hpp"

#include <pp_protocol/command.hpp>

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    X_GUARD(DeviceSelectorService, ServiceIoContext);
    X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(DSDDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(DeviceObserver, ServiceIoContext);

    DSRDownloader.OnUpdateDeviceStateRelayServerListCallback = [](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & DSRInfo) {
        auto OSL = std::vector<xNetAddress>();
        for (auto & S : DSRInfo) {
            OSL.push_back(S.ObserverAddress);
        }
        DeviceObserver.UpdateServerList(OSL);
    };
    DSDDownloader.OnUpdateDeviceSelectorDispatcherServerListCallback = [](uint32_t Version, const std::vector<xDeviceSelectorDispatcherInfo> & ServerList) {
        DeviceSelectorService.UpdateDispatcherList(ServerList);
    };
    DeviceObserver.OnPacketCallback = [](const xMessageChannel & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) -> bool {
        switch (CommandId) {
            case Cmd_DSR_DS_DeviceUpdate: {
                auto PP = xPP_DeviceInfoUpdate();
                if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
                    DEBUG_LOG("Invalid device info");
                    return true;
                }

                if (!PP.IsOffline) {
                    DEBUG_LOG("DeviceHeartbeat");
                    auto LocalDevInfo                    = xDR_DeviceInfoBase{};
                    LocalDevInfo.DeviceId                = PP.DeviceUuid;
                    LocalDevInfo.ReleayServerRuntimeId   = PP.RelayServerRuntimeId;
                    LocalDevInfo.RelayServerSideDeviceId = PP.RelayServerSideDeviceId;

                    LocalDevInfo.CountryId = PP.CountryId;
                    LocalDevInfo.StateId   = PP.StateId;
                    LocalDevInfo.CityId    = PP.CityId;

                    DeviceContextManager.UpdateDevice(LocalDevInfo);
                } else {
                    DEBUG_LOG("DeviceOffline");
                    DeviceContextManager.RemoveDeviceById(PP.DeviceUuid);
                }
                return true;
            }

            default:
                DEBUG_LOG("Unknown CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
                break;
        }
        return true;
    };

    auto AuditTickRunner = xTickRunner(60'000, [](uint64_t) {
        AuditLogger->I("%s", LocalAudit.ToString().c_str());
        LocalAudit.ResetPeriodCount();
    });
    while (true) {
        ServiceUpdateOnce(DeviceSelectorService, DSRDownloader, DSDDownloader, DeviceObserver, DeviceContextManager, AuditTickRunner);
    }
    return 0;
}
