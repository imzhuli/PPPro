#include "../lib_server_list/device_state_relay_server_list_downloader.hpp"
#include "../lib_server_util/all.hpp"

#include <functional>
#include <pp_common/_.hpp>
#include <pp_protocol/ad_bk/device_info.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/device_state.hpp>
#include <unordered_set>

static auto ServerListDownloadAddress = xNetAddress();

static auto KR                  = xKfkProducer();
static auto SecurityProtocol    = std::string();
static auto SaslMechanism       = std::string();
static auto SaslUsername        = std::string();
static auto SaslPassword        = std::string();
static auto BootstrapServerList = std::string();
static auto Topic               = std::string();

struct xODI_DeviceKeepAliveNode : xListNode {};
using xODI_DeviceKeepAliveList = xList<xODI_DeviceKeepAliveNode>;

struct xODI_DeviceInfo : xODI_DeviceKeepAliveNode {

    std::string DeviceUuid;
    uint64_t    OnlineTimestampMS        = 0;
    uint64_t    LastKeepAliveTimestampMS = 0;

    uint32_t    Version;
    xNetAddress PrimaryIpv4Address;
    xNetAddress PrimaryIpv6Address;
    uint64_t    TotalUploadSizeSinceOnline;    // 指发向目标的数据
    uint64_t    TotalDownloadSizeSinceOnline;  // 下载数据
    uint32_t    CurrentConnectionCount;
    uint32_t    CurrentUdpChannelCount;

    bool SupportUdpChannel;
    bool SupportDnsRequests;
    bool SpeedLimitEnabled;

    uint32_t TotalNewConnectionsSinceLastPost;
    uint32_t TotalClosedConnectionSinceLastPost;
    uint32_t TotalNewUdpChannelSinceLastPost;
    uint32_t TotalClosedUdpChannelSinceLastPost;
    uint32_t TotalDnsRequestSinceLastPost;
};

static auto DeviceMap         = std::unordered_map<std::string, xODI_DeviceInfo>();
static auto DeviceTimeoutList = xODI_DeviceKeepAliveList();
static auto DeviceObserver    = xClientPoolWrapper();
static auto DSRDownloader     = xDeviceStateRelayServerListDownloader();

static void PostDeviceInfo(const xODI_DeviceInfo * DP, bool Online) {
    DEBUG_LOG("DeviceId: %s, Online=%s, Ip=%s, Ipv6=%s", DP->DeviceUuid.c_str(), YN(Online), DP->PrimaryIpv4Address.ToString().c_str(), DP->PrimaryIpv6Address.ToString().c_str());

    auto   Req   = xAD_BK_ReportDeviceInfoSingle();
    auto & ReqDI = Req.DeviceInfo;

    auto NowMS = ServiceTicker();

    Req.LocalAuditTimestampMS = NowMS;

    ReqDI.Version    = DP->Version;
    ReqDI.DeviceUuid = DP->DeviceUuid;
    // ReqDI.RelayServerUuid    = DP->RelayServerUuid;
    ReqDI.PrimaryIpv4Address = DP->PrimaryIpv4Address;
    ReqDI.PrimaryIpv6Address = DP->PrimaryIpv6Address;

    ReqDI.IsOffline          = !Online;
    ReqDI.SupportUdpChannel  = DP->SupportUdpChannel;
    ReqDI.SupportDnsRequests = DP->SupportDnsRequests;
    ReqDI.SpeedLimitEnabled  = DP->SpeedLimitEnabled;

    ReqDI.TotalOnlineTimeMS = NowMS - DP->OnlineTimestampMS;

    ubyte Buffer[MaxPacketSize];
    auto  MSize = WriteMessage(Buffer, Cmd_AuditTerminalInfo2, 0, Req);

    auto MsgKey = DP->DeviceUuid;
    KR.Post(MsgKey, Buffer, MSize);

    DEBUG_LOG("\n%s", HexShow(Buffer, MSize).c_str());
}

static void OnDeviceUpdate(ubyte * PayloadPtr, size_t PayloadSize) {
    auto PP = xPP_DeviceInfoUpdate();
    if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("Invalid device info");
        return;
    }

    if (PP.IsOffline) {
        auto Iter = DeviceMap.find(PP.DeviceUuid);
        if (Iter == DeviceMap.end()) {
            DEBUG_LOG("Found offline device");
            return;
        }
        PostDeviceInfo(&Iter->second, false);
        DeviceMap.erase(Iter);
        return;
    }

    auto   NowMS = ServiceTicker();
    auto & DI    = DeviceMap[PP.DeviceUuid];

    DI.DeviceUuid        = PP.DeviceUuid;
    DI.OnlineTimestampMS = NowMS;

    DI.Version            = PP.Version;
    DI.PrimaryIpv4Address = PP.PrimaryIpv4Address;
    DI.PrimaryIpv6Address = PP.PrimaryIpv6Address;

    DI.SupportUdpChannel  = PP.SupportUdpChannel;
    DI.SupportDnsRequests = PP.SupportDnsRequests;
    DI.SpeedLimitEnabled  = PP.SpeedLimitEnabled;

    // KeepAlive
    PostDeviceInfo(&DI, true);
    DI.LastKeepAliveTimestampMS = NowMS;
    DeviceTimeoutList.GrabTail(DI);
}

static bool OnServerPacket(xMessagePoster * Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    DEBUG_LOG("CommandId=% " PRIx32 "\n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
    switch (CommandId) {
        case Cmd_DSR_DS_DeviceUpdate:
            OnDeviceUpdate(PayloadPtr, PayloadSize);
            break;
    }

    return true;
}

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);

    auto CL = RuntimeEnv.LoadConfig();
    CL.Require(SecurityProtocol, "SecurityProtocol");
    CL.Require(SaslMechanism, "SaslMechanism");
    CL.Require(SaslUsername, "SaslUsername");
    CL.Require(SaslPassword, "SaslPassword");
    CL.Require(BootstrapServerList, "BootstrapServerList");
    CL.Require(Topic, "Topic");
    CL.Require(ServerListDownloadAddress, "ServerListDownloadAddress");

    auto BootstrapServersOpt = ParsePythonStringArray(BootstrapServerList);
    RuntimeAssert(BootstrapServersOpt);
    auto KfkBootstrapServers = JoinStr(*BootstrapServersOpt, ",");
    DEBUG_LOG("KfkBootstrapServers: %s", KfkBootstrapServers.c_str());

    RuntimeAssert(KR.Init(
        Topic,
        {
            { "security.protocol", SecurityProtocol },
            { "sasl.mechanism", SaslMechanism },
            { "sasl.username", SaslUsername },
            { "sasl.password", SaslPassword },
            { "bootstrap.servers", KfkBootstrapServers },
        }
    ));
    auto KRC = xScopeCleaner(KR);

    X_GUARD(DeviceObserver, ServiceIoContext);
    X_GUARD(DSRDownloader, ServiceIoContext, ServerListDownloadAddress);
    DSRDownloader.SetUpdateDeviceStateRelayServerListCallback([](const std::vector<xDeviceStateRelayServerInfo> & NewServerList) {
        auto SL = std::vector<xNetAddress>();
        for (auto & I : NewServerList) {
            SL.push_back(I.ObserverAddress);
        }
        DeviceObserver.UpdateServerList(SL);
    });
    DeviceObserver.SetOnPacketCallback(OnServerPacket);

    while (true) {
        ServiceUpdateOnce(DeviceObserver, DSRDownloader);
    }

    return 0;
}
