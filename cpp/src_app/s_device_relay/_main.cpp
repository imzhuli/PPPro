#include "./_global.hpp"
#include "./relay_service.hpp"

#include <pp_protocol/device_relay/init_ctrl_stream.hpp>

int main(int argc, char ** argv) {

    auto SEG = xRuntimeEnvGuard(argc, argv);

    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(BindCtrlAddress, "BindCtrlAddress");
    CL.Require(BindDataAddress, "BindDataAddress");
    CL.Require(BindProxyAddress, "BindProxyAddress");
    CL.Require(ExportCtrlAddress, "ExportCtrlAddress");
    CL.Require(ExportDataAddress, "ExportDataAddress");
    CL.Require(ExportProxyAddress, "ExportProxyAddress");
    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    RuntimeAssert(DeviceReporter.Init(ServiceIoContext));
    RuntimeAssert(DeviceManager.Init(MaxDeviceCount));
    RuntimeAssert(DeviceConnectionManager.Init(ServiceIoContext, MaxDeviceCount * 2));
    RuntimeAssert(DeviceRelayService.Init(ServiceIoContext, BindCtrlAddress, BindDataAddress, BindProxyAddress));
    RuntimeAssert(ProxyConnectionManager.Init(ServiceIoContext, MaxProxyCount));

    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RelayConnectionManager, MaxRelayConnectionCount);
    X_GUARD(RIDDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(RelayInfoReporter, ServiceIoContext);

    ServerIdClient.SetOnServerIdUpdateCallback([&](auto ServerId) {
        ServerRuntimeId = ServerId;
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, ServerRuntimeId);

        auto LocalInfo                    = xRelayServerInfoBase();
        LocalInfo.ServerId                = ServerRuntimeId;
        LocalInfo.StartupTimestampMS      = ServiceTicker();
        LocalInfo.ServerType              = eRelayServerType::DEVICE;
        LocalInfo.ExportDeviceCtrlAddress = ExportCtrlAddress;
        LocalInfo.ExportDeviceDataAddress = ExportDataAddress;
        LocalInfo.ExportProxyAddress      = ExportProxyAddress;
        RelayInfoReporter.UpdateLocalRelayServerInfo(LocalInfo);
    });

    RIDDownloader.SetOnUpdateRelayInfoDispatcherServerInfoCallback([](const xRelayInfoDispatcherServerInfo & Info) {
        DEBUG_LOG("RelayInfoDispatcher producer address updated: %s", Info.ProducerAddress.ToString().c_str());
        RelayInfoReporter.UpdateServerAddress(Info.ProducerAddress);
    });
    DSRDownloader.SetOnUpdateDeviceStateRelayServerListCallback([](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & ServerList) {
        auto PSL = std::vector<xNetAddress>();
        for (auto & S : ServerList) {
            PSL.push_back(S.ProducerAddress);
        }
        DeviceReporter.UpdateServerList(PSL);
    });
    DeviceReporter.SetOnUpdateServerListCallback([](const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed) {
        auto OS = std::ostringstream();
        for (const auto & A : Added) {
            OS << "A:" << A.ToString() << " ";
        }
        for (const auto & R : Removed) {
            OS << "R:" << R.ToString() << " ";
        }
        DEBUG_LOG("%s", OS.str().c_str());
    });

    auto AuditTimer = xTimer();
    while (true) {
        ServiceUpdateOnce(
            DeviceConnectionManager,  //
            DeviceManager,            //
            DeviceRelayService,       //
            ProxyConnectionManager,   //
            RelayConnectionManager,   //
            DeviceReporter,           //
            ServerIdClient,           //
            RIDDownloader,            //
            DSRDownloader,            //
            RelayInfoReporter
        );
        if (AuditTimer.TestAndTag(std::chrono::minutes(1))) {
            Logger->I("%s", LocalAudit.ToString().c_str());
            LocalAudit.ResetPeriodicalValues();
        }
    }

    return 0;
}
