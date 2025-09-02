#include "../lib_utils/all.hpp"

struct xD_OutputAddress {
    xNetAddress BindV4;
    xNetAddress BindV6;

    xNetAddress ExportV4;
    xNetAddress ExportV6;
};

struct xD_TestAddressServerAddress {
    xNetAddress V4;
    xNetAddress V6;
};

static xNetAddress    GainedAddress4       = {};
static xNetAddress    GainedAddress6       = {};
static bool           FinishedTestAddress4 = false;
static bool           FinishedTestAddress6 = false;
static xClientWrapper Ipv4TestClient;
static xClientWrapper Ipv6TestClient;
static xUdpService    Ipv4UdpTestClient;
static xUdpService    Ipv6UdpTestClient;

static auto OutputAddress            = xD_OutputAddress();
static auto TestAddressServerAddress = xD_TestAddressServerAddress();

bool OnServerPush(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_DV_RL_AddressPush) {
        return false;
    }
    auto R = xPP_AddressAutoPush();

    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("invalid protocol");
        return false;
    }
    Logger->I("(tcp)ServerPushedAddress: %s", R.ConnectionAddress.ToString().c_str());
    if (R.ConnectionAddress.Is4()) {
        GainedAddress4 = R.ConnectionAddress;
    } else if (R.ConnectionAddress.Is6()) {
        GainedAddress6 = R.ConnectionAddress;
    }

    return true;
}

void OnUdpServerPush(const xUdpServiceChannelHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    // DEBUG_LOG("CID=%" PRIx64 ", RID=%" PRIx64 "\n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());

    if (CommandId != Cmd_DV_RL_AddressPush) {
        return;
    }
    auto R = xPP_AddressAutoPush();

    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("invalid protocol");
        return;
    }
    Logger->I("(udp)ServerPushedAddress: %s", R.ConnectionAddress.ToString().c_str());
    return;
}

int main(int argc, char ** argv) {
    auto REG = xRuntimeEnvGuard(argc, argv);
    auto CL  = RuntimeEnv.LoadConfig();

    CL.Optional(OutputAddress.BindV4, "OutputBindAddress4");
    CL.Optional(OutputAddress.BindV6, "OutputBindAddress6");
    CL.Optional(TestAddressServerAddress.V4, "TestAddressServerAddress4");
    CL.Optional(TestAddressServerAddress.V6, "TestAddressServerAddress6");

    auto EnableV4 = OutputAddress.BindV4 && TestAddressServerAddress.V4;
    auto EnableV6 = OutputAddress.BindV6 && TestAddressServerAddress.V6;

    if (EnableV4) {
        Logger->I("ipv4 enabled");
        RuntimeAssert(Ipv4TestClient.Init(ServiceIoContext));
        Ipv4TestClient.UpdateTarget(TestAddressServerAddress.V4);
        Ipv4TestClient.OnPacketCallback = OnServerPush;

        RuntimeAssert(Ipv4UdpTestClient.Init(ServiceIoContext, xNetAddress::Make4()));
        Ipv4UdpTestClient.OnPacketCallback = OnUdpServerPush;
        auto UdpChallenge                  = xPP_AddressChallenge();
        Ipv4UdpTestClient.PostMessage(TestAddressServerAddress.V4, Cmd_DV_RL_AddressChallenge, 0, UdpChallenge);
    }
    if (EnableV6) {
        Logger->I("ipv6 enabled");
        RuntimeAssert(Ipv6TestClient.Init(ServiceIoContext));
        Ipv6TestClient.UpdateTarget(TestAddressServerAddress.V6);
        Ipv6TestClient.OnPacketCallback = OnServerPush;

        RuntimeAssert(Ipv6UdpTestClient.Init(ServiceIoContext, xNetAddress::Make6()));
        Ipv6UdpTestClient.OnPacketCallback = OnUdpServerPush;
        auto UdpChallenge                  = xPP_AddressChallenge();
        Ipv6UdpTestClient.PostMessage(TestAddressServerAddress.V6, Cmd_DV_RL_AddressChallenge, 0, UdpChallenge);
    }

    while (ServiceRunState) {
        ServiceUpdateOnce();
        if (EnableV4 && !FinishedTestAddress4) {
            if (!GainedAddress4) {
                Ipv4TestClient.Tick(ServiceTicker());
            } else {
                FinishedTestAddress4 = true;
                Ipv4TestClient.Clean();
            }
        }
        if (EnableV6 && !FinishedTestAddress6) {
            if (!GainedAddress6) {
                Ipv6TestClient.Tick(ServiceTicker());
            } else {
                FinishedTestAddress6 = true;
                Ipv6TestClient.Clean();
            }
        }
    }

    if (EnableV4) {
        Ipv4UdpTestClient.Clean();
        if (!FinishedTestAddress4) {
            Ipv4TestClient.Clean();
        }
    }

    if (EnableV6) {
        Ipv6UdpTestClient.Clean();
        if (!FinishedTestAddress6) {
            Ipv6TestClient.Clean();
        }
    }
    return 0;
}
