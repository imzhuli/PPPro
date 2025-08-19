#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "../lib_server_list/device_selector_dispatcher_list_downloader.hpp"
#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_server_list/relay_info_dispatcher_server_info_downloader.hpp"
#include "../lib_utils/all.hpp"
#include "./file_dump.hpp"

static auto ServerListDownloadAddress = xNetAddress();
static auto AADownloader              = xAuditAccountServerListDownloader();
static auto ACDownloader              = xAuthCacheServerListDownloader();
static auto DSRDownloader             = xDeviceStateRelayServerListDownloader();
static auto RIDDownloader             = xRelayInfoDispatcherServerInfoDownloader();
static auto DSDDownloader             = xDeviceSelectorDispatcherServerListDownloader();

static auto RelayInfoWatcher = xClientWrapper();

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");
    Logger->D("DownloadAddress: %s", ServerListDownloadAddress.ToString().c_str());

    X_GUARD(RelayInfoWatcher, ServiceIoContext);
    RelayInfoWatcher.OnPacketCallback = [](xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) -> bool {
        if (CommandId != Cmd_BroadcastRelayInfo) {
            DEBUG_LOG("invalid command id");
            return true;
        }
        auto Msg = xPP_BroadcastRelayInfo();
        if (!Msg.Deserialize(PayloadPtr, PayloadSize)) {
            DEBUG_LOG("invalid protocol");
            return true;
        }
        Logger->I("RelayServerInfo: %s", Msg.ServerInfo.ToString().c_str());
        return true;
    };

    // X_GUARD(AADownloader, ServiceIoContext, ServerListDownloadAddress);
    // AADownloader.SetOnUpdateAuditAccountServerListCallback([](uint32_t Version, const std::vector<xServerInfo> & ServerList) {
    //     auto OS = std::ostringstream();
    //     OS << "updated audit account server list: version=" << Version << endl;
    //     for (auto S : ServerList) {
    //         OS << "ServerId: " << S.ServerId << endl;
    //         OS << "ServerAddress: " << S.Address.ToString() << endl;
    //     }
    //     Logger->D("%s", OS.str().c_str());
    // });

    // X_GUARD(ACDownloader, ServiceIoContext, ServerListDownloadAddress);
    // ACDownloader.SetOnUpdateAuthCacheServerListCallback([](uint32_t Version, const std::vector<xServerInfo> & ServerList) {
    //     auto OS = std::ostringstream();
    //     OS << "updated auth cache server list: version=" << Version << endl;
    //     for (auto S : ServerList) {
    //         OS << "ServerId: " << S.ServerId << endl;
    //         OS << "ServerAddress: " << S.Address.ToString() << endl;
    //     }
    //     Logger->D("%s", OS.str().c_str());
    // });

    // X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    // DSRDownloader.SetOnUpdateDeviceStateRelayServerListCallback([](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & ServerList) {
    //     auto OS = std::ostringstream();
    //     OS << "updated device state relay server list: version=" << Version << endl;
    //     for (auto S : ServerList) {
    //         OS << "ServerId: " << S.ServerId << endl;
    //         OS << "ServerAddress: P:" << S.ProducerAddress.ToString() << ", O: " << S.ObserverAddress.ToString() << endl;
    //     }
    //     Logger->D("%s", OS.str().c_str());
    // });

    X_GUARD(RIDDownloader, ServiceIoContext, ServerListDownloadAddress);
    RIDDownloader.OnUpdateServerInfoCallback = [](const xRelayInfoDispatcherServerInfo & S) {  //
        Logger->D("%s", S.ToString().c_str());
        DumpToFile(RuntimeEnv.GetCachePath("dump.relay_info_dispatcher.txt"), S.ToString());

        RelayInfoWatcher.UpdateTarget(S.ObserverAddress4);
    };

    // X_GUARD(DSDDownloader, ServiceIoContext, ServerListDownloadAddress);
    // DSDDownloader.SetOnUpdateDeviceSelectorDispatcherServerListCallback([](uint32_t Version, const std::vector<xDeviceSelectorDispatcherInfo> & SL) {
    //     auto OS = std::ostringstream();
    //     OS << "updated device selector dispatcher server: version=" << Version << endl;
    //     for (auto & S : SL) {
    //         OS << "ServerId: " << S.ServerId << endl;
    //         OS << "ServerAddress: P:" << S.ExportAddressForClient.ToString() << ", O: " << S.ExportAddressForServiceProvider.ToString() << endl;
    //     }
    //     Logger->D("%s", OS.str().c_str());
    // });

    while (ServiceRunState) {
        ServiceUpdateOnce(
            // AADownloader, ACDownloader, DSRDownloader,
            RIDDownloader, RelayInfoWatcher,
            // DSDDownloader
            DeadTicker
        );
    }
}
