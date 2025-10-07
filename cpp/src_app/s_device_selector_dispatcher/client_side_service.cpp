#include "./client_side_service.hpp"

#include "./_global.hpp"

bool OnDSDClientSidePacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    DEBUG_LOG("");
    switch (CommandId) {
        case Cmd_DeviceSelector_AcquireDevice:
            return OnDeviceSelect(Handle, RequestId, PayloadPtr, PayloadSize);
        default:
            break;
    }
    return true;
}

bool OnDeviceSelect(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_AcquireDevice();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("Invalid protocol");
        return false;
    }

    auto Server = ServiceProviderManager.SelectServiceProvider(R.DevicePoolId, R.StrategyFlags);
    if (!Server) {
        DEBUG_LOG("no service provider, PoolId=%" PRIx32 ", StrategyFlags=%" PRIx16 "", R.DevicePoolId, R.StrategyFlags);
        auto FailedResp = xPP_AcquireDeviceResp();
        Handle.PostMessage(Cmd_DeviceSelector_AcquireDeviceResp, RequestId, FailedResp);
        return true;
    }

    auto ServiceConnectionHandle = xTcpServiceClientConnectionHandle(&ServerSideService, Server->ConnectionId);
    DEBUG_LOG(
        "relay request to target, SourceConnectionId=%" PRIx64 ", ServerId=%" PRIx64 ", ServerConnectionId=%" PRIx64 "",
        //
        Handle.GetConnectionId(), Server->ServiceProviderId, Server->ConnectionId
    );

    if (!ServiceConnectionHandle.operator->()) {
        DEBUG_LOG("missing connection, ConnectionId=%" PRIx64 "", Server->ConnectionId);
        auto FailedResp = xPP_AcquireDeviceResp();
        Handle.PostMessage(Cmd_DeviceSelector_AcquireDeviceResp, RequestId, FailedResp);
        return true;
    }

    auto RCP = RequestContextPool.Acquire({ .U64 = Handle.GetConnectionId() }, { .U64 = RequestId });
    if (!RCP) {
        DEBUG_LOG("failed to acquire request context");
        auto FailedResp = xPP_AcquireDeviceResp();
        Handle.PostMessage(Cmd_DeviceSelector_AcquireDeviceResp, RequestId, FailedResp);
        return true;
    }

    ServiceConnectionHandle.PostMessage(Cmd_DeviceSelector_AcquireDevice, RCP->RequestId, R);
    return true;
}
