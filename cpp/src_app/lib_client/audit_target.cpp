#include "./audit_target.hpp"

#include <pp_protocol/command.hpp>

static constexpr const uint64_t RequestTimeoutMS = 2'000;
static constexpr const size_t   MaxRequestCount  = 10'0000;

bool xAuditTargetClient::Init(xIoContext * ICP, const xel::xNetAddress & ServerListAddress) {
    if (!ATSD.Init(ICP, ServerListAddress)) {
        return false;
    }
    if (!CPW.Init(ICP)) {
        ATSD.Clean();
        return false;
    }

    ATSD.OnUpdateAuditTargetServerListCallback = [this](uint32_t Version, const std::vector<xServerInfo> & ServerList) {
        std::vector<xNetAddress> AL;
        for (auto S : ServerList) {
            AL.push_back(S.Address);
        }
        std::sort(AL.begin(), AL.end());
        CPW.UpdateServerList(AL);
    };

    CPW.OnConnectedCallback = [this](auto &) {
        if (!CPWConnections++) {
            OnEnabled();
        }
    };
    CPW.OnDisonnectedCallback = [this](auto &) {
        if (!--CPWConnections) {
            OnDisabled();
        }
    };

    return true;
}

void xAuditTargetClient::Tick(uint64_t NowMS) {
    T.Update(NowMS);
    ATSD.Tick(NowMS);
    CPW.Tick(NowMS);
}

void xAuditTargetClient::Clean() {
    Reset(CPWConnections);
    CPW.Clean();
    ATSD.Clean();
}

bool xAuditTargetClient::Request(uint64_t AuditId, const std::string_view & Target, uint64_t Count) {
    Todo("Report ");
    return true;
}
