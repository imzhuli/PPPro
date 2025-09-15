#pragma once
#include <pp_common/_.hpp>

struct xDR_DeviceContext final {
    uint64_t                          Id;
    xTcpServiceClientConnectionHandle Handle;

    // runtime:
    std::string Uuid;
    uint64_t    StartupTimestampMS;
    uint32_t    Version;
    uint32_t    ChannelId;
    xDeviceFlag Flags;
};

extern void InitDeviceContextManager();
extern void CleanDeviceContextManager();

extern xDR_DeviceContext * GetDeviceContextById(uint64_t Id);
extern bool OnDeviceConnectionPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CmdId, xPacketRequestId ReqId, ubyte * PayloadPtr, size_t PayloadSize);
extern void OnDeviceConnectionClean(const xTcpServiceClientConnectionHandle & Handle);
