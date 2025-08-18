#include "../lib_utils/all.hpp"

#include <pp_protocol/ad_bk/usage_by_audit_id.hpp>
#include <pp_protocol/command.hpp>

static constexpr size_t MAX_AUDIT_REPORTER_COUNT = 2'0000;

static auto BindAddress               = xNetAddress();
static auto ExportServerAddress       = xNetAddress();
static auto ServerIdCenterAddress     = xNetAddress();
static auto ServerListRegisterAddress = xNetAddress();

static auto KR                  = xKfkProducer();
static auto SecurityProtocol    = std::string();
static auto SaslMechanism       = std::string();
static auto SaslUsername        = std::string();
static auto SaslPassword        = std::string();
static auto BootstrapServerList = std::string();
static auto Topic               = std::string();

auto ServerIdClient       = xServerIdClient();
auto RegisterServerClient = xRegisterServerClient();

static void AA_RegisterServer(const xMessageChannel & Poster, uint64_t LocalServerId) {
    auto Req     = xPP_RegisterAuditAccountServer();
    Req.ServerId = LocalServerId;
    Req.Address  = ExportServerAddress;
    Poster.PostMessage(Cmd_RegisterAuditAccountServer, 0, Req);
}

struct xAuditAccountInfoNode
    : xListNode
    , xAuditAccountInfo {
    uint64_t LastReportTimestampMS = {};
    uint64_t CollectorCounter      = {};
};
using xAuditAccountList = xList<xAuditAccountInfoNode>;

std::string ToString(const xAuditAccountInfoNode & Info) {
    auto OS = std::ostringstream();
    OS << "AuditId: " << Info.AuditId << ' ';
    OS << "CollectorCounter: " << Info.CollectorCounter << ' ';
    OS << "TotalUploadSize: " << Info.TotalUploadSize << ' ';
    OS << "TotalDownloadSize: " << Info.TotalDownloadSize << ' ';
    return OS.str();
}

static void PostAuditAccoungUsage(xAuditAccountInfoNode & Info) {
    DEBUG_LOG("ReportAccountInfo: %s", ToString(Info).c_str());

    auto R             = xAD_BK_ReportUsageByAuditList();
    R.LocalTimestampMS = GetTimestampMS();

    auto A                            = xAD_BK_UsageByAuditId();
    A.AuditId                         = Info.AuditId;
    A.TotalTcpConnectionSinceLastPost = Steal(Info.TotalTcpCount);
    A.TotalUdpChannelSinceLastPost    = Steal(Info.TotalUdpCount);
    A.TotalUploadSizeSinceLastPost    = Steal(Info.TotalUploadSize);
    A.TotalDownloadSizeSinceLastPost  = Steal(Info.TotalDownloadSize);
    R.AuditList.push_back(A);

    ubyte Buffer[MaxPacketSize];
    auto  MSize = WriteMessage(Buffer, Cmd_AuditUsageByAuditId, 0, R);

    auto MsgKey = std::to_string(A.AuditId);
    KR.Post(MsgKey, Buffer, MSize);

    DEBUG_LOG("\n%s", HexShow(Buffer, MSize).c_str());
}

class xAuditService : public xService {
public:
    static constexpr const uint64_t MAX_REPORT_TIMEOUT_MS = 5 * 60'000;

    using xService::Clean;
    using xService::Init;
    using xService::Tick;

private:
    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        switch (CommandId) {
            case Cmd_AuditAccountUsage:
                return OnAuditAccountUsage(PayloadPtr, PayloadSize);
            case Cmd_AuditAccountTarget:
                return OnAuditAccountTarget(PayloadPtr, PayloadSize);
            default:
                break;
        }

        return true;
        //
    }

private:
    void OnTick(uint64_t NowMS) override {
        uint64_t          ReportStartTimepoint = NowMS - MAX_REPORT_TIMEOUT_MS;
        xAuditAccountList TempList;
        while (auto P = AuditReportQueue.PopHead([NowMS, ReportStartTimepoint, &TempList](const xAuditAccountInfoNode & N) {
            return N.LastReportTimestampMS <= ReportStartTimepoint;
        })) {
            P->LastReportTimestampMS = NowMS;
            TempList.AddTail(*P);

            PostAuditAccoungUsage(*P);
        }
        AuditReportQueue.GrabListTail(TempList);
    }

    bool OnAuditAccountUsage(ubyte * PayloadPtr, size_t PayloadSize) {
        auto R = xAuditAccountUsage();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            DEBUG_LOG("invalid protocol");
            return false;
        }
        auto & Node = AuditMap[R.AuditId];
        if (!Node.AuditId) {  // new node
            Node.AuditId               = R.AuditId;
            Node.LastReportTimestampMS = GetTickTimeMS();
            AuditReportQueue.AddTail(Node);
        }
        ++Node.CollectorCounter;
        Node.TotalUploadSize   += R.UploadSize;
        Node.TotalDownloadSize += R.DownloadSize;
        Node.TotalTcpCount     += R.TcpIncreament;
        Node.TotalUdpCount     += R.UdpIncreament;

        // test:
        DEBUG_LOG("%s", ToString(Node).c_str());
        if (Node.CollectorCounter) {
            ForceQueueFirst(Node);
        }
        return true;  //
    }

    void ForceQueueFirst(xAuditAccountInfoNode & Info) {
        Info.LastReportTimestampMS = 0;
        AuditReportQueue.GrabHead(Info);
    }

    bool OnAuditAccountTarget(ubyte * PayloadPtr, size_t PayloadSize) {
        return true;  //
    }

    std::unordered_map<uint64_t /* AuditId */, xAuditAccountInfoNode> AuditMap;
    xAuditAccountList                                                 AuditReportQueue;
};

static xAuditService AuditService;

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Require(BindAddress, "BindAddress");
    CL.Require(ExportServerAddress, "ExportServerAddress");
    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");

    CL.Require(SecurityProtocol, "SecurityProtocol");
    CL.Require(SaslMechanism, "SaslMechanism");
    CL.Require(SaslUsername, "SaslUsername");
    CL.Require(SaslPassword, "SaslPassword");
    CL.Require(BootstrapServerList, "BootstrapServerList");
    CL.Require(Topic, "Topic");

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

    X_GUARD(ServerIdClient, ServiceIoContext, ServerIdCenterAddress, RuntimeEnv.DefaultLocalServerIdFilePath);
    X_GUARD(RegisterServerClient, ServiceIoContext, ServerListRegisterAddress);
    X_GUARD(AuditService, ServiceIoContext, BindAddress, MAX_AUDIT_REPORTER_COUNT);

    ServerIdClient.OnServerIdUpdateCallback = [](uint64_t LocalServerId) {
        DumpLocalServerId(RuntimeEnv.DefaultLocalServerIdFilePath, LocalServerId);
        RegisterServerClient.SetLocalServerId(LocalServerId);
    };
    RegisterServerClient.SetServerIdPoster(&AA_RegisterServer);

    while (ServiceRunState) {
        ServiceUpdateOnce(ServerIdClient, RegisterServerClient, AuditService);
    }

    return 0;
}
