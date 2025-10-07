#include "../lib_client/device_selector_client.hpp"
#include "../lib_utils/service_common.hpp"

static auto DSC = xDeviceSelectorClient();

int main(int argc, char ** argv) {
    X_INSTANCE xRuntimeEnvGuard(argc, argv, false);
    auto       CL = xCommandLine{
        argc,
        argv,
              {
            { 's', nullptr, "server_list_server", true },
        },
    };
    auto OS = CL["server_list_server"];
    RuntimeAssert(OS);

    X_GUARD(DSC, ServiceIoContext, xNetAddress::Parse(*OS));

    DSC.OnEnabled = []() {
        cout << "Enabled" << endl;
        auto Ops           = xDeviceSelectorOptions();
        Ops.StrategyFlags |= DSS_IPV4;
        DSC.Request(1024, Ops);
    };

    DSC.OnDeviceSelectResultCallback = [](uint64_t RequestContextId, const xDeviceSelectorResult & Result) {
        cout << "RelayId: " << Result.DeviceRelayServerRuntimeId << endl;
        cout << "DeviceId: " << Result.DeviceRelaySideId << endl;
        ServiceRunState.Stop();
    };

    while (ServiceRunState) {
        ServiceUpdateOnce(DSC);
    }

    return 0;
}
