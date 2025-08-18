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

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    ServerIdClient.OnServerIdUpdateCallback = [&](auto ServerId) {};
    RIDDownloader.UpdateServerInfoCallback  = [](const xRelayInfoDispatcherServerInfo & Info) {
        DEBUG_LOG("RelayInfoDispatcher producer address updated: %s", Info.ToString().c_str());
        Todo("using correct target address");
        // RelayInfoReporter.UpdateServerAddress(Info.ProducerAddress);
    };
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

    while (true) {
        ServiceUpdateOnce(ServerIdClient  //
                                          // DeviceConnectionManager,  //
                                          // DeviceManager,            //
                                          // DeviceRelayService,       //
                                          // ProxyConnectionManager,   //
                                          // RelayConnectionManager,   //
                                          // DeviceReporter,           //
                                          // RIDDownloader,            //
                                          // DSRDownloader,            //
                                          // RelayInfoReporter
        );
    }

    return 0;
}
