#include "./server_side_service.hpp"

#include "./_global.hpp"

bool xDSD_ServerSideService::OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    switch (CommandId) {
        case Cmd_RegisterDeviceSelector:
            return OnRegisterServiceProvider(Connection, RequestId, PayloadPtr, PayloadSize);
    }
    //
    return true;
}

void xDSD_ServerSideService::OnClientClose(xServiceClientConnection & Connection) {
    auto ServiceProviderId = Steal(Connection.UserContext.U64);
    if (!ServiceProviderId) {
        return;
    }
    ServiceProviderManager.RemoveServer(ServiceProviderId);
    --LocalAudit.CurrentServiceProviderCount;
}

bool xDSD_ServerSideService::OnRegisterServiceProvider(xServiceClientConnection & Connection, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_RegisterDeviceSelector();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }

    auto PreviousServerId = Connection.UserContext.U64;
    if (PreviousServerId) {
        DEBUG_LOG("duplicate server info");
        ++LocalAudit.TotalDuplicateRegistation;
        return false;
    }

    if (!ServiceProviderManager.AddServer(Connection.GetConnectionId(), R.ServerInfo)) {
        ++LocalAudit.TotalAddServerInfoError;
        auto Resp = xPP_RegisterServerResp();
        PostMessage(Connection, Cmd_RegisterServerResp, RequestId, Resp);
        return true;
    }
    ++LocalAudit.CurrentServiceProviderCount;

    auto Resp     = xPP_RegisterServerResp();
    Resp.Accepted = True;
    PostMessage(Connection, Cmd_RegisterServerResp, RequestId, Resp);
    return true;
}