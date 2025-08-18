#include "../lib_utils/all.hpp"
#include "./global.hpp"
#include "./producer.hpp"
#include "./service_register.hpp"

// struct xRID_ProducerService : public xService {

// public:
//     using xRelayServerKeepAliveCallback = std::function<void(const xRelayServerInfoBase &)>;
//     void SetRelayServerKeepAliveCallback(const xRelayServerKeepAliveCallback & CB) { OnRelayServerKeepAliveCallback = CB; }

//     void OnClientConnected(xServiceClientConnection & Connection) override { ++LocalAudit.TotalRelayConnections; }
//

// private:
//     xRelayServerKeepAliveCallback OnRelayServerKeepAliveCallback;
// };

// struct xRID_ConsumerService : xService {

//     bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
//         if (CommandId != Cmd_RegisterRelayInfoObserver) {
//             Logger->E("invalid challenge");
//             return false;
//         }

//         auto & P = Connection.UserContext.P;
//         if (P) {
//             Logger->E("duplicate registration");
//             return false;
//         }
//         auto R = xPP_RegisterRelayInfoObserver();
//         if (!R.Deserialize(PayloadPtr, PayloadSize)) {
//             Logger->E("duplicate protocol");
//             return false;
//         }

//         auto InsertPoint = std::lower_bound(SortedAvailableObserverConnections.begin(), SortedAvailableObserverConnections.end(), &Connection);
//         assert(InsertPoint == SortedAvailableObserverConnections.end() || *InsertPoint != &Connection);
//         SortedAvailableObserverConnections.insert(InsertPoint, &Connection);
//         P = &Connection;

//         ++LocalAudit.CurrentObserverCount;
//         return true;
//     }

//     void OnClientClose(xServiceClientConnection & Connection) override {
//         auto & P = Connection.UserContext.P;
//         if (!P) {
//             return;
//         }

//         assert(&Connection == P);
//         auto CheckPoint = std::lower_bound(SortedAvailableObserverConnections.begin(), SortedAvailableObserverConnections.end(), &Connection);
//         assert(CheckPoint != SortedAvailableObserverConnections.end() || *CheckPoint == &Connection);
//         SortedAvailableObserverConnections.erase(CheckPoint);

//         --LocalAudit.CurrentObserverCount;
//     }

// private:
//     std::vector<xServiceClientConnection *> SortedAvailableObserverConnections;
// };

static auto RSS = xRID_RegisterServerService();
// static auto PS  = xRID_ProducerService();
// static auto OS  = xRID_ConsumerService();

static bool Enable4 = false;
static bool Enable6 = false;

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    // extern xNetAddress ProducerAddress4;
    // extern xNetAddress ProducerAddress6;
    // extern xNetAddress ObserverAddress4;
    // extern xNetAddress ObserverAddress6;
    extern xNetAddress ExportProducerAddress4;
    extern xNetAddress ExportProducerAddress6;
    extern xNetAddress ExportObserverAddress4;
    extern xNetAddress ExportObserverAddress6;

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");

    CL.Require(ProducerAddress4, "ProducerAddress4");
    CL.Require(ObserverAddress4, "ObserverAddress4");
    CL.Require(ProducerAddress6, "ProducerAddress6");
    CL.Require(ObserverAddress6, "ObserverAddress6");

    CL.Optional(ExportProducerAddress4, "ExportProducerAddress4");
    CL.Optional(ExportObserverAddress4, "ExportObserverAddress4");
    CL.Optional(ExportProducerAddress6, "ExportProducerAddress6");
    CL.Optional(ExportObserverAddress6, "ExportObserverAddress6");

    Enable4 = ProducerAddress4.IsV4() && ProducerAddress4.Port && ObserverAddress4.IsV4() && ObserverAddress4.Port;
    Enable6 = ProducerAddress6.IsV6() && ProducerAddress6.Port && ObserverAddress6.IsV6() && ObserverAddress6.Port;
    Logger->I("Enable4=%s, Enable6=%s", YN(Enable4), YN(Enable6));

    if (!Enable4 && !Enable6) {
        Logger->F("neither ipv4 or ipv6 is enabled");
        return 0;
    }

    X_GUARD(RSS);

    ProducerService4.OnClientPacket    = OnProducerClientPacket;
    ProducerService4.OnClientKeepAlive = OnProducerClientKeepAlive;
    ProducerService4.OnClientClose     = OnProducerClientClose;
    X_COND_GUARD(Enable4, ProducerService4, ServiceIoContext, ProducerAddress4, MAX_DEVICE_RELAY_SERVER_SUPPORTED);
    X_COND_GUARD(Enable4, ObserverService4, ServiceIoContext, ObserverAddress4, DEFAULT_MAX_SERVER_CONNECTIONS);

    ProducerService6.OnClientPacket    = OnProducerClientPacket;
    ProducerService6.OnClientKeepAlive = OnProducerClientKeepAlive;
    ProducerService6.OnClientClose     = OnProducerClientClose;

    X_COND_GUARD(Enable6, ProducerService6, ServiceIoContext, ProducerAddress6, MAX_DEVICE_RELAY_SERVER_SUPPORTED);
    X_COND_GUARD(Enable6, ObserverService6, ServiceIoContext, ObserverAddress6, DEFAULT_MAX_SERVER_CONNECTIONS);

    while (ServiceRunState) {
        ServiceUpdateOnce(RSS, LocalAuditLogger);
        if (Enable4) {
            TickAll(ServiceTicker(), ProducerService4, ObserverService4);
        }
        if (Enable6) {
            TickAll(ServiceTicker(), ProducerService6, ObserverService6);
        }
    }

    return 0;

    //
}