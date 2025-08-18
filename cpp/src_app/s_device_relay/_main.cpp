#include "./_global.hpp"

#include <pp_protocol/device_relay/init_ctrl_stream.hpp>

int main(int argc, char ** argv) {

    auto SEG = xRuntimeEnvGuard(argc, argv);

    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Optional(BindAddress4, "BindAddress4");
    CL.Optional(BindAddress6, "BindAddress6");
    CL.Optional(ProxyAddress4, "ProxyAddress4");
    CL.Optional(ProxyAddress6, "ProxyAddress6");
    CL.Optional(ExportAddress4, "ExportAddress4");
    CL.Optional(ExportAddress6, "ExportAddress6");
    CL.Optional(ExportProxyAddress4, "ExportProxyAddress4");
    CL.Optional(ExportProxyAddress6, "ExportProxyAddress6");

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    bool Enable4 = BindAddress4.IsV4() && BindAddress4.Port;
    bool Enable6 = BindAddress6.IsV6() && BindAddress6.Port;
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

        LocalInfo.ExportDeviceAddress4 = ExportAddress4;
        LocalInfo.ExportDeviceAddress6 = ExportAddress6;

        RIReporter.UpdateLocalRelayServerInfo(LocalInfo);
    };
    RIDDownloader.OnUpdateServerInfoCallback = [=](const xRelayInfoDispatcherServerInfo & Info) {
        DEBUG_LOG("RelayInfoDispatcher producer address updated: %s", Info.ToString().c_str());
        if (Enable4 && Info.ProducerAddress4.IsV4()) {
            RIReporter.UpdateServerAddress(Info.ProducerAddress4);
        } else if (Enable6 && Info.ProducerAddress6.IsV6()) {
            RIReporter.UpdateServerAddress(Info.ProducerAddress6);
        } else {
            Logger->E("invalid relay info dispatcher address support!");
        }
    };

    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RIDDownloader, ServiceIoContext, ServerListDownloadAddress);
    X_GUARD(RIReporter, ServiceIoContext);

    // DSRDownloader.SetOnUpdateDeviceStateRelayServerListCallback([](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & ServerList) {
    //     auto PSL = std::vector<xNetAddress>();
    //     for (auto & S : ServerList) {
    //         PSL.push_back(S.ProducerAddress);
    //     }
    //     DeviceReporter.UpdateServerList(PSL);
    // });
    // DeviceReporter.SetOnUpdateServerListCallback([](const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed) {
    //     auto OS = std::ostringstream();
    //     for (const auto & A : Added) {
    //         OS << "A:" << A.ToString() << " ";
    //     }
    //     for (const auto & R : Removed) {
    //         OS << "R:" << R.ToString() << " ";
    //     }
    //     DEBUG_LOG("%s", OS.str().c_str());
    // });

    while (true) {
        ServiceUpdateOnce(
            ServerIdClient,  //
                             // DeviceConnectionManager,  //
                             // DeviceManager,            //
                             // DeviceRelayService,       //
                             // ProxyConnectionManager,   //
                             // RelayConnectionManager,   //
                             // DeviceReporter,           //
            RIDDownloader,   //
            // DSRDownloader,            //
            RIReporter
        );
    }

    return 0;
}
