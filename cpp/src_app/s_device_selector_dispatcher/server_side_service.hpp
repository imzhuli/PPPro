#pragma once
#include <pp_common/_.hpp>

extern bool OnDSDServerSidePacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
extern void OnDSDServerSideClose(const xTcpServiceClientConnectionHandle & Handle);

extern bool OnRegisterServiceProvider(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnAcquireDeviceResp(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
