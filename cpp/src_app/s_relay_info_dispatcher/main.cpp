#include "../lib_utils/all.hpp"
#include "./global.hpp"
#include "./service_register.hpp"

static struct xLocalAudit {
    size_t TotalRelayConnections          = 0;
    size_t TotalAvailableRelayConnections = 0;
    size_t TotalRegisteredConnections     = 0;
    size_t TotalClosedConnections         = 0;

    size_t CurrentObserverCount = 0;
} LocalAudit;

#define O(x) OS << #x << "=" << LocalAudit.x << endl
std::string LocalAuditToString() {
    auto OS = std::ostringstream();
    O(TotalRelayConnections);
    O(TotalAvailableRelayConnections);
    O(TotalRegisteredConnections);
    O(TotalClosedConnections);

    O(CurrentObserverCount);
    return OS.str();
}
#undef O

static struct xRID_LocalAuditLogger {

    static constexpr const uint64_t LocalAuditTimeoutMS = 60'000;

    void Tick(uint64_t NowMS) {
        if (NowMS - LastLogTimestampMS < LocalAuditTimeoutMS) {
            return;
        }
        LastLogTimestampMS = NowMS;
        AuditLogger->I("%s", LocalAuditToString().c_str());
    }

    uint64_t LastLogTimestampMS;
} LocalAuditLogger;

struct xRID_ProducerService : public xService {

public:
    using xRelayServerKeepAliveCallback = std::function<void(const xRelayServerInfoBase &)>;
    void SetRelayServerKeepAliveCallback(const xRelayServerKeepAliveCallback & CB) { OnRelayServerKeepAliveCallback = CB; }

    void OnClientConnected(xServiceClientConnection & Connection) override { ++LocalAudit.TotalRelayConnections; }
    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        // DEBUG_LOG("\n%s", HexShow(PayloadPtr, PayloadSize).c_str());

        auto & P = Connection.UserContext.P;
        if (P) {
            DEBUG_LOG("duplicate server registration");
            return false;
        }

        if (CommandId != Cmd_RelayServerHeartBeat) {
            DEBUG_LOG("invalid command id");
            return false;
        }

        auto R = xPP_RelayServerHeartBeat();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            DEBUG_LOG("invalid protocol base");
            return false;
        }

        auto InfoCopy = new xRelayServerInfoBase(R.ServerInfo);
        P             = InfoCopy;
        ++LocalAudit.TotalRegisteredConnections;
        ++LocalAudit.TotalAvailableRelayConnections;
        if (OnRelayServerKeepAliveCallback) {
            Logger->I("New relay server info: %s", InfoCopy->ToString().c_str());
            OnRelayServerKeepAliveCallback(*InfoCopy);
        }
        return true;
    }

    void OnClientKeepAlive(xServiceClientConnection & Connection) {
        auto InfoCopy = static_cast<xRelayServerInfoBase *>(Connection.UserContext.P);
        if (!InfoCopy) {
            return;
        }
        if (OnRelayServerKeepAliveCallback) {
            OnRelayServerKeepAliveCallback(*InfoCopy);
        }
    }

    void OnClientClose(xServiceClientConnection & Connection) override {
        ++LocalAudit.TotalClosedConnections;
        auto InfoCopy = static_cast<xRelayServerInfoBase *>(Steal(Connection.UserContext.P));
        if (!InfoCopy) {
            return;
        }
        delete InfoCopy;
        --LocalAudit.TotalAvailableRelayConnections;
    }

private:
    xRelayServerKeepAliveCallback OnRelayServerKeepAliveCallback;
};

struct xRID_ConsumerService : xService {

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        if (CommandId != Cmd_RegisterRelayInfoObserver) {
            Logger->E("invalid challenge");
            return false;
        }

        auto & P = Connection.UserContext.P;
        if (P) {
            Logger->E("duplicate registration");
            return false;
        }
        auto R = xPP_RegisterRelayInfoObserver();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            Logger->E("duplicate protocol");
            return false;
        }

        auto InsertPoint = std::lower_bound(SortedAvailableObserverConnections.begin(), SortedAvailableObserverConnections.end(), &Connection);
        assert(InsertPoint == SortedAvailableObserverConnections.end() || *InsertPoint != &Connection);
        SortedAvailableObserverConnections.insert(InsertPoint, &Connection);
        P = &Connection;

        ++LocalAudit.CurrentObserverCount;
        return true;
    }

    void OnClientClose(xServiceClientConnection & Connection) override {
        auto & P = Connection.UserContext.P;
        if (!P) {
            return;
        }

        assert(&Connection == P);
        auto CheckPoint = std::lower_bound(SortedAvailableObserverConnections.begin(), SortedAvailableObserverConnections.end(), &Connection);
        assert(CheckPoint != SortedAvailableObserverConnections.end() || *CheckPoint == &Connection);
        SortedAvailableObserverConnections.erase(CheckPoint);

        --LocalAudit.CurrentObserverCount;
    }

    void BroadCastRelayInfo(const xRelayServerInfoBase & Info) {
        auto Msg       = xPP_BroadcastRelayInfo();
        Msg.ServerInfo = Info;

        ubyte B[MaxPacketSize];
        auto  RS = WriteMessage(B, Cmd_BroadcastRelayInfo, 0, Msg);

        for (auto & P : SortedAvailableObserverConnections) {
            PostData(*P, B, RS);
        }
    }

private:
    std::vector<xServiceClientConnection *> SortedAvailableObserverConnections;
};

static auto RSS = xRID_RegisterServerService();
static auto PS  = xRID_ProducerService();
static auto OS  = xRID_ConsumerService();

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);

    auto CL = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");
    CL.Require(ProducerAddress, "ProducerAddress");
    CL.Require(ObserverAddress, "ObserverAddress");
    CL.Require(ExportProducerAddress, "ExportProducerAddress");
    CL.Require(ExportObserverAddress, "ExportObserverAddress");

    X_GUARD(RSS);
    X_GUARD(PS, ServiceIoContext, ProducerAddress, MAX_DEVICE_RELAY_SERVER_SUPPORTED, true);
    X_GUARD(OS, ServiceIoContext, ObserverAddress, DEFAULT_MAX_SERVER_CONNECTIONS, true);

    PS.SetRelayServerKeepAliveCallback([](auto & Info) { OS.BroadCastRelayInfo(Info); });

    while (ServiceRunState) {
        ServiceUpdateOnce(RSS, PS, OS, LocalAuditLogger);
    }

    return 0;

    //
}