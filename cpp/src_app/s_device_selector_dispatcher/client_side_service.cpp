#include "./client_side_service.hpp"

#include "./_global.hpp"

bool xDSD_ClientSideService::OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    switch (CommandId) {
        case Cmd_DeviceSelector_AcquireDevice:
            return OnDeviceSelect(Connection, RequestId, PayloadPtr, PayloadSize);
        default:
            break;
    }

    return true;
}

bool xDSD_ClientSideService::OnDeviceSelect(xServiceClientConnection & Connection, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_AcquireDevice();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("Invalid protocol");
        return false;
    }

    // try select server:
    DEBUG_LOG("NOT PROCESSED");
    return true;
}
