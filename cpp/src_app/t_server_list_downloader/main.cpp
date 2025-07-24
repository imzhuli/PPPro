#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_server_list/relay_info_dispatcher_server_info_downloader.hpp"
#include "../lib_server_util/all.hpp"

static auto ServerListDownloadAddress = xNetAddress();
static auto ACDownloader              = xAuthCacheServerListDownloader();
static auto DSRDownloader             = xDeviceStateRelayServerListDownloader();
static auto RIDDownloader             = xRelayInfoDispatcherServerInfoDownloader();

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    X_GUARD(ACDownloader, ServiceIoContext, ServerListDownloadAddress);
    ACDownloader.SetUpdateAuthCacheServerListCallback([](const std::vector<xServerInfo> & ServerList) {
        auto OS = std::ostringstream();
        OS << "Updatedd auth cache server list: " << endl;
        for (auto S : ServerList) {
            OS << "ServerId: " << S.ServerId << endl;
            OS << "ServerAddress: " << S.Address.ToString() << endl;
        }
        DEBUG_LOG("%s", OS.str().c_str());
    });

    X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    DSRDownloader.SetUpdateDeviceStateRelayServerListCallback([](const std::vector<xDeviceStateRelayServerInfo> & ServerList) {
        auto OS = std::ostringstream();
        OS << "Updatedd device state relay server list: " << endl;
        for (auto S : ServerList) {
            OS << "ServerId: " << S.ServerId << endl;
            OS << "ServerAddress: P:" << S.ProducerAddress.ToString() << ", O: " << S.ObserverAddress.ToString() << endl;
        }
        DEBUG_LOG("%s", OS.str().c_str());
    });

    X_GUARD(RIDDownloader, ServiceIoContext, ServerListDownloadAddress);
    RIDDownloader.SetUpdateRelayInfoDispatcherServerInfoCallback([](const xRelayInfoDispatcherServerInfo & S) {
        auto OS = std::ostringstream();
        OS << "Updatedd relay info dispatcher server: " << endl;
        OS << "ServerId: " << S.ServerId << endl;
        OS << "ServerAddress: P:" << S.ProducerAddress.ToString() << ", O: " << S.ObserverAddress.ToString() << endl;
        DEBUG_LOG("%s", OS.str().c_str());
    });

    while (ServiceRunState) {
        ServiceUpdateOnce(ACDownloader, DSRDownloader, RIDDownloader);
    }
}
