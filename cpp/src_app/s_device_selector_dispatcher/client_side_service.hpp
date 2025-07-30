#pragma once
#include <pp_common/_.hpp>

class xDSD_ClientSideService
    : xService
    , xSingleton<xDSD_ClientSideService> {
public:
    using xService::Clean;
    using xService::Init;
    using xService::Tick;

    //
private:
    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    bool OnDeviceSelect(xServiceClientConnection & Connection, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

    //
};