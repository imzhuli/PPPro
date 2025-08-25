#pragma once
#include <pp_common/_.hpp>

struct xDR_DeviceContext final {
    uint64_t                          Id;
    xTcpServiceClientConnectionHandle Handle;
};

extern xIndexedStorage<xDR_DeviceContext> DeviceManager;
extern bool OnDeviceConnectionPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CmdId, xPacketRequestId ReqId, ubyte * PayloadPtr, size_t PayloadSize);
extern void OnDeviceConnectionClose(const xTcpServiceClientConnectionHandle & Handle);
