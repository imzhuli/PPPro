#include "./client_side_service.hpp"

#include "./_global.hpp"

bool OnDSDClientSidePacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
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

    // try select server:
    DEBUG_LOG("NOT PROCESSED");
    return true;
}
