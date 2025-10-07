#include "./server_side_service.hpp"

#include "./_global.hpp"

bool OnDSDServerSidePacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_RegisterDeviceSelector:
            return OnRegisterServiceProvider(Handle, RequestId, PayloadPtr, PayloadSize);
        case Cmd_DeviceSelector_AcquireDeviceResp:
            return OnAcquireDeviceResp(Handle, RequestId, PayloadPtr, PayloadSize);
    }
    //
    return true;
}

void OnDSDServerSideClose(const xTcpServiceClientConnectionHandle & Handle) {
    auto ServiceProviderId = Steal(Handle->UserContext.U64);
    DEBUG_LOG("ServiceProviderId: %" PRIx64 "", ServiceProviderId);
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

    auto SPId = ServiceProviderManager.AddServer(Handle.GetConnectionId(), R.ServerInfo);
    if (!SPId) {
        ++LocalAudit.TotalAddServerInfoError;
        auto Resp = xPP_RegisterServerResp();
        Handle.PostMessage(Cmd_RegisterServerResp, RequestId, Resp);
        return true;
    }
    Handle->UserContext.U64 = SPId;
    ++LocalAudit.CurrentServiceProviderCount;

    auto Resp     = xPP_RegisterServerResp();
    Resp.Accepted = True;
    Handle.PostMessage(Cmd_RegisterServerResp, RequestId, Resp);
    Logger->I(
        "ServiceProvider accepted, ServerConnectionId=%" PRIx64 ", ServerPoolId=%" PRIx32 ", StrategyFlags=%" PRIx16 "",  //
        Handle.GetConnectionId(),                                                                                         //
        R.ServerInfo.DevicePoolId,                                                                                        //
        R.ServerInfo.StrategyFlags                                                                                        //
    );
    return true;
}

bool OnAcquireDeviceResp(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    DEBUG_LOG("");

    auto R = xPP_AcquireDeviceResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }

    auto RCP = RequestContextPool.CheckAndGet(RequestId);
    if (!RCP) {
        DEBUG_LOG("request not match");
        return true;
    }
    X_AT_EXIT([RCP] { RequestContextPool.Release(RCP); });

    auto ConnectionId     = RCP->RequestContext.U64;
    auto SourceRequestId  = RCP->RequestContextEx.U64;
    auto ConnectionHandle = xTcpServiceClientConnectionHandle(&ClientSideService, ConnectionId);
    if (!ConnectionHandle.operator->()) {
        DEBUG_LOG("missing original connection");
        return true;
    }

    DEBUG_LOG("post response: DeviceRelayServerRuntimeId=%" PRIx64 ", DeviceRelaySideId=%" PRIx64 "", R.DeviceRelayServerRuntimeId, R.DeviceRelaySideId);
    ConnectionHandle.PostMessage(Cmd_DeviceSelector_AcquireDeviceResp, SourceRequestId, R);
    return true;
}
