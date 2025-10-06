#include "../lib_utils/all.hpp"
#include "./global.hpp"
#include "./producer.hpp"
#include "./service_register.hpp"

static auto RSS     = xRID_RegisterServerService();
static bool Enable4 = false;

int main(int argc, char ** argv) {

    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Require(ServerIdCenterAddress, "ServerIdCenterAddress");
    CL.Require(ServerListRegisterAddress, "ServerListRegisterAddress");

    CL.Require(ProducerAddress4, "ProducerAddress4");
    CL.Require(ObserverAddress4, "ObserverAddress4");

    CL.Optional(ExportProducerAddress4, "ExportProducerAddress4");
    CL.Optional(ExportObserverAddress4, "ExportObserverAddress4");

    Enable4 = ProducerAddress4.Is4() && ProducerAddress4.Port && ObserverAddress4.Is4() && ObserverAddress4.Port;
    if (!Enable4) {
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

    while (ServiceRunState) {
        ServiceUpdateOnce(RSS, LocalAuditLogger, ProducerService4, ObserverService4);
    }

    return 0;

    //
}