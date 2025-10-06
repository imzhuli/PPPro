#include "./_global.hpp"

int main(int argc, char ** argv) {

    auto SEG = xRuntimeEnvGuard(argc, argv);

    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Optional(DeviceAddress4, "DeviceAddress4");
    CL.Optional(DeviceAddress6, "DeviceAddress6");
    CL.Optional(ProxyAddress4, "ProxyAddress4");
    CL.Optional(ProxyAddress6, "ProxyAddress6");
    CL.Optional(ExportDeviceAddress4, "ExportDeviceAddress4");
    CL.Optional(ExportDeviceAddress6, "ExportDeviceAddress6");
    CL.Optional(ExportProxyAddress4, "ExportProxyAddress4");
    CL.Optional(ExportProxyAddress6, "ExportProxyAddress6");

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    bool Enable4 = DeviceAddress4.Is4() && DeviceAddress4.Port && ProxyAddress4.Is4() && ProxyAddress4.Port;
    bool Enable6 = DeviceAddress6.Is6() && DeviceAddress6.Port && ProxyAddress6.Is6() && ProxyAddress6.Port;
    if (!Enable4 && !Enable6) {
        Logger->F("neither ipv4 or ipv6 is enabled");
        return 0;
    }

    ServerIdClient.OnServerIdUpdateCallback = [&](auto ServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, ServerId);

        auto LocalInfo               = xRelayServerInfoBase();
        LocalInfo.ServerType         = eRelayServerType::DEVICE;
        LocalInfo.ServerId           = ServerId;
        LocalInfo.StartupTimestampMS = ServiceTicker();

        LocalInfo.ExportProxyAddress4 = ExportProxyAddress4;
        LocalInfo.ExportProxyAddress6 = ExportProxyAddress6;

        LocalInfo.ExportDeviceAddress4 = ExportDeviceAddress4;
        LocalInfo.ExportDeviceAddress6 = ExportDeviceAddress6;

        RIReporter.UpdateLocalRelayServerInfo(LocalInfo);
    };

    RIDDownloader.OnUpdateServerInfoCallback = [=](const xRelayInfoDispatcherServerInfo & Info) {
        DEBUG_LOG("RelayInfoDispatcher producer address updated: %s", Info.ToString().c_str());
        RIReporter.UpdateServerAddress(Info.ProducerAddress4);
    };

    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RIDDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(RIReporter, ServiceIoContext);
    X_GUARD(DeviceReporter, ServiceIoContext);

    auto X_VAR = xel::xScopeGuard(InitDeviceContextManager, CleanDeviceContextManager);
    X_COND_GUARD(Enable4, DeviceService4, ServiceIoContext, DeviceAddress4, MaxDeviceCount);
    X_COND_GUARD(Enable4, ProxyService4, ServiceIoContext, ProxyAddress4, MaxDeviceCount);
    X_COND_GUARD(Enable6, DeviceService6, ServiceIoContext, DeviceAddress6, MaxDeviceCount);
    X_COND_GUARD(Enable6, ProxyService6, ServiceIoContext, ProxyAddress6, MaxDeviceCount);

    auto X_VAR = xel::xScopeGuard([] { InitRelayContextPool(MaxRelayContextCount); }, CleanRelayContextPool);

    DeviceService6.OnClientPacket = DeviceService4.OnClientPacket = &OnDeviceConnectionPacket;
    DeviceService6.OnClientClean = DeviceService4.OnClientClean = &OnDeviceConnectionClean;
    DeviceService6.OnClientKeepAlive = DeviceService4.OnClientKeepAlive = &OnDeviceKeepAlive;

    DSRDownloader.OnUpdateDeviceStateRelayServerListCallback = [](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & ServerList) {
        auto PSL = std::vector<xNetAddress>();
        for (auto & S : ServerList) {
            PSL.push_back(S.ProducerAddress);
        }
        DeviceReporter.UpdateServerList(PSL);
    };
    DeviceReporter.OnUpdateServerListCallback = [](const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed) {
        auto OS = std::ostringstream();
        for (const auto & A : Added) {
            OS << "A:" << A.ToString() << " ";
        }
        for (const auto & R : Removed) {
            OS << "R:" << R.ToString() << " ";
        }
        DEBUG_LOG("%s", OS.str().c_str());
    };

    auto Auditer = xTickRunner(60'000, [](uint64_t) { AuditLogger->I("%s", LocalAudit.ToString().c_str()); });
    while (true) {
        ServiceUpdateOnce(
            ServerIdClient,  //
                             // DeviceConnectionManager,  //
                             // DeviceManager,            //
                             // DeviceRelayService,       //
                             // ProxyConnectionManager,   //
                             // RelayConnectionManager,   //
            RIDDownloader,   //
            DSRDownloader,   //
            RIReporter,      //
            DeviceReporter,  //
            Auditer,         //
            DeadTicker
        );
        if (Enable4) {
            TickAll(ServiceTicker(), DeviceService4, ProxyService4);
        }
        if (Enable6) {
            TickAll(ServiceTicker(), DeviceService6, ProxyService6);
        }
    }

    return 0;
}
