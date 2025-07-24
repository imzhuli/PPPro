#include "../lib_server_util/all.hpp"

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

    while (ServiceRunState) {
        ServiceUpdateOnce();
    }

    return 0;
}
