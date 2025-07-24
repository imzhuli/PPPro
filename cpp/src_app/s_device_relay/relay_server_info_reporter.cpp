#include "./relay_server_info_reporter.hpp"

bool xRelayInfoReporter::Init(xIoContext * ICP) {
    RuntimeAssert(Client.Init(ICP));
    Client.SetOnConnectedCallback([this]() {
        if (!LocalRelayServerInfo.ServerId) {
            return;
        }
        auto R       = xPP_RelayServerHeartBeat();
        R.ServerInfo = LocalRelayServerInfo;
        Client.PostMessage(Cmd_RelayServerHeartBeat, 0, R);
    });
    return true;
}

void xRelayInfoReporter::Clean() {
    Client.Clean();
    Reset(LocalRelayServerInfo);
    return;
}

void xRelayInfoReporter::Tick(uint64_t NowMS) {
    Client.Tick(NowMS);
}

void xRelayInfoReporter::UpdateServerAddress(const xNetAddress & Address) {
    Client.UpdateTarget(Address);
}

void xRelayInfoReporter::UpdateLocalRelayServerInfo(const xRelayServerInfoBase & Info) {
    LocalRelayServerInfo = Info;
    Client.Kill();
}
