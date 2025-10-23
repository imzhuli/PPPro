#include "../lib_server_list/audit_account_server_list_downloader.hpp"
#include "../lib_server_list/audit_target_server_list_downloader.hpp"
#include "../lib_server_list/auth_cache_server_list_downloader.hpp"
#include "../lib_server_list/backend_server_list_downloader.hpp"
#include "../lib_server_list/device_selector_dispatcher_list_downloader.hpp"
#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_server_list/relay_info_dispatcher_server_info_downloader.hpp"
#include "../lib_server_list/relay_info_observer.hpp"
#include "../lib_utils/all.hpp"
#include "./file_dump.hpp"

static auto ServerListDownloadAddress = xNetAddress();

static auto AADownloader = xAuditAccountServerListDownloader();
static auto AAReady      = false;

static auto ACDownloader = xAuthCacheServerListDownloader();
static auto ACReady      = false;

static auto ATDownlolader = xAuditTargetServerListDownloader();
static auto ATReady       = false;

static auto DSRDownloader = xDeviceStateRelayServerListDownloader();
static auto DSRReady      = false;

static auto DSDDownloader = xDeviceSelectorDispatcherServerListDownloader();
static auto DSDReady      = false;

static auto BSDownloader = xBackendServerListDownloader();
static auto BSDReady     = false;

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");
    printf("DownloadAddress: %s", ServerListDownloadAddress.ToString().c_str());

    X_GUARD(AADownloader, ServiceIoContext, ServerListDownloadAddress);
    AADownloader.OnUpdateAuditAccountServerListCallback = [](uint32_t Version, const std::vector<xServerInfo> & ServerList) {
        if (Steal(AAReady, true)) {
            return;
        }
        auto OS = std::ostringstream();
        OS << "updated audit account server list: version=" << Version << endl;
        for (auto S : ServerList) {
            OS << "ServerId: " << S.ServerId << endl;
            OS << "ServerAddress: " << S.Address.ToString() << endl;
        }
        cout << OS.str() << endl;
    };

    X_GUARD(ACDownloader, ServiceIoContext, ServerListDownloadAddress);
    ACDownloader.OnUpdateAuthCacheServerListCallback = [](uint32_t Version, const std::vector<xServerInfo> & ServerList) {
        if (Steal(ACReady, true)) {
            return;
        }
        auto OS = std::ostringstream();
        OS << "updated auth cache server list: version=" << Version << endl;
        for (auto S : ServerList) {
            OS << "ServerId: " << S.ServerId << endl;
            OS << "ServerAddress: " << S.Address.ToString() << endl;
        }
        cout << OS.str() << endl;
    };

    X_GUARD(ATDownlolader, ServiceIoContext, ServerListDownloadAddress);
    ATDownlolader.OnUpdateAuditTargetServerListCallback = [](uint32_t Version, const std::vector<xServerInfo> & ServerList) {
        if (Steal(ATReady, true)) {
            return;
        }
        auto OS = std::ostringstream();
        OS << "updated audit target server list: version=" << Version << endl;
        for (auto S : ServerList) {
            OS << "ServerId: " << S.ServerId << endl;
            OS << "ServerAddress: " << S.Address.ToString() << endl;
        }
        cout << OS.str() << endl;
    };

    X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    DSRDownloader.OnUpdateDeviceStateRelayServerListCallback = [](uint32_t Version, const std::vector<xDeviceStateRelayServerInfo> & ServerList) {
        if (Steal(DSRReady, true)) {
            return;
        }
        auto OS = std::ostringstream();
        OS << "updated device state relay server list: version=" << Version << endl;
        for (auto S : ServerList) {
            OS << "ServerId: " << S.ServerId << endl;
            OS << "ServerAddress: P:" << S.ProducerAddress.ToString() << ", O: " << S.ObserverAddress.ToString() << endl;
        }
        cout << OS.str() << endl;
    };

    X_GUARD(DSDDownloader, ServiceIoContext, ServerListDownloadAddress);
    DSDDownloader.OnUpdateDeviceSelectorDispatcherServerListCallback = [](uint32_t Version, const std::vector<xDeviceSelectorDispatcherInfo> & SL) {
        if (Steal(DSDReady, true)) {
            return;
        }
        auto OS = std::ostringstream();
        OS << "updated device selector dispatcher server: version=" << Version << endl;
        for (auto & S : SL) {
            OS << "ServerId: " << S.ServerId << endl;
            OS << "ServerAddress: P:" << S.ExportAddressForClient.ToString() << ", O: " << S.ExportAddressForServiceProvider.ToString() << endl;
        }
        cout << OS.str() << endl;
    };

    X_GUARD(BSDownloader, ServiceIoContext, ServerListDownloadAddress);
    BSDownloader.OnUpdateCallback = [](uint32_t Version, const std::vector<xNetAddress> & FullList, const std::vector<xNetAddress> & Added,
                                       const std::vector<xNetAddress> & Removed) {
        if (Steal(BSDReady, true)) {
            return;
        }
        auto OS = std::ostringstream();
        OS << "updated backend server list, version=" << Version << endl;
        for (auto & S : FullList) {
            OS << "Address: " << S.ToString() << endl;
        }
        cout << OS.str() << endl;
    };

    auto Timeout = xTimer();
    while (Timeout.Elapsed() < 5s && ServiceRunState && !(AAReady && ACReady && ATReady && DSRReady && DSDReady && BSDReady)) {
        ServiceUpdateOnce(
            AADownloader,   //
            ACDownloader,   //
            ATDownlolader,  //
            DSRDownloader,  //
            DSDDownloader,  //
            BSDownloader,   //
            DeadTicker
        );
    }

    cout << "AAReady = " << YN(AAReady) << endl;
    cout << "ACReady = " << YN(ACReady) << endl;
    cout << "ATReady = " << YN(ATReady) << endl;
    cout << "DSRReady = " << YN(DSRReady) << endl;
    cout << "DSDReady = " << YN(DSDReady) << endl;
    cout << "BSDReady = " << YN(BSDReady) << endl;
}
