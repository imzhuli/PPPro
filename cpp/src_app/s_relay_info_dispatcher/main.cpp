#include "../lib_utils/all.hpp"
#include "./global.hpp"
#include "./producer.hpp"
#include "./service_register.hpp"

static auto RSS     = xRID_RegisterServerService();
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

    Enable4 = ProducerAddress4.Is4() && ProducerAddress4.Port && ObserverAddress4.Is4() && ObserverAddress4.Port;
    Enable6 = ProducerAddress6.Is6() && ProducerAddress6.Port && ObserverAddress6.Is6() && ObserverAddress6.Port;
    Logger->I("Enable4=%s, Enable6=%s", YN(Enable4), YN(Enable6));

    if (!Enable4 && !Enable6) {
        Logger->F("neither ipv4 or ipv6 is enabled");
        return 0;
    }

    X_GUARD(RSS);

    ProducerService4.OnClientPacket    = OnProducerClientPacket;
    ProducerService4.OnClientKeepAlive = OnProducerClientKeepAlive;
    ProducerService4.OnClientClose     = OnProducerClientClose;

    ObserverService4.OnClientConnected = OnObserverClientConnected;
    ObserverService4.OnClientClose     = OnObserverClientClose;

    X_COND_GUARD(Enable4, ProducerService4, ServiceIoContext, ProducerAddress4, MAX_DEVICE_RELAY_SERVER_SUPPORTED);
    X_COND_GUARD(Enable4, ObserverService4, ServiceIoContext, ObserverAddress4, DEFAULT_MAX_SERVER_CONNECTIONS);

    ProducerService6.OnClientPacket    = OnProducerClientPacket;
    ProducerService6.OnClientKeepAlive = OnProducerClientKeepAlive;
    ProducerService6.OnClientClose     = OnProducerClientClose;

    ObserverService6.OnClientConnected = OnObserverClientConnected;
    ObserverService6.OnClientClose     = OnObserverClientClose;

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