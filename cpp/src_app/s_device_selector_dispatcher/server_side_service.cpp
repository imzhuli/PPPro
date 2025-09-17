#include "./server_side_service.hpp"

#include "./_global.hpp"

bool OnDSDServerSidePacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_RegisterDeviceSelector:
            return OnRegisterServiceProvider(Handle, RequestId, PayloadPtr, PayloadSize);
    }
    //
    return true;
}

void OnDSDServerSideClose(const xTcpServiceClientConnectionHandle & Handle) {
    auto ServiceProviderId = Steal(Handle->UserContext.U64);
    if (!ServiceProviderId) {
        return;
    }
    ServiceProviderManager.RemoveServer(ServiceProviderId);
    --LocalAudit.CurrentServiceProviderCount;
}

bool OnRegisterServiceProvider(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_RegisterDeviceSelector();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }

    auto PreviousServerId = Handle->UserContext.U64;
    if (PreviousServerId) {
        DEBUG_LOG("duplicate server info");
        ++LocalAudit.TotalDuplicateRegistation;
        return false;
    }

    if (!ServiceProviderManager.AddServer(Handle.GetConnectionId(), R.ServerInfo)) {
        ++LocalAudit.TotalAddServerInfoError;
        auto Resp = xPP_RegisterServerResp();
        Handle.PostMessage(Cmd_RegisterServerResp, RequestId, Resp);
        return true;
    }
    ++LocalAudit.CurrentServiceProviderCount;

    auto Resp     = xPP_RegisterServerResp();
    Resp.Accepted = True;
    Handle.PostMessage(Cmd_RegisterServerResp, RequestId, Resp);
    return true;
}