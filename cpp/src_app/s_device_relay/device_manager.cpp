#include "./device_manager.hpp"

#include "../lib_utils/all.hpp"
#include "pp_protocol/device_relay/connection.hpp"

#include <pp_protocol/device_relay/handshake.hpp>

xIndexedStorage<xDR_DeviceContext> DeviceManager;

//////

static bool OnDeviceHandshake(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DeviceHandshake();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("invalid protocol");
        return false;
    }
    auto Id = DeviceManager.Acquire({ 0, Handle });
    if (!Id) {
        return false;
    }

    auto & DC = DeviceManager[Id];

    Handle->UserContext.U64 = DC.Id = Id;

    auto RS     = xPP_DeviceHandshakeResp();
    RS.Accepted = true;
    Handle.PostMessage(Cmd_DV_RL_HandshakeResp, 0, RS);

    // // test
    // auto CC                  = xPP_CreateConnectionHost();
    // CC.RelaySideConnectionId = 1024;
    // CC.HostnameView          = "www.baidu.com";
    // CC.Port                  = 80;
    // Handle.PostMessage(Cmd_DV_RL_CreateConnectionHost, 0, CC);

    return true;
}

void OnDeviceConnectionClose(const xTcpServiceClientConnectionHandle & Handle) {
    auto Id = Handle->UserContext.U64;
    if (!Id) {
        DEBUG_LOG("remove uninited device connection");
        return;
    }
    DEBUG_LOG("remove handshake ready device connection: %" PRIx64 "", Id);
    DeviceManager.Release(Id);
}

bool OnDeviceConnectionPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CmdId, xPacketRequestId ReqId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Id = Handle->UserContext.U64;
    if (!Id) {
        DEBUG_LOG("uninited connection, require registration");
        if (CmdId != Cmd_DV_RL_Handshake) {
            Logger->E("invalid handshake");
            return false;
        }
        return OnDeviceHandshake(Handle, PayloadPtr, PayloadSize);
    }
    DeviceManager.Release(Id);
    return true;
}
