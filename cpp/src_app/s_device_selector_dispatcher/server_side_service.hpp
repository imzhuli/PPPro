#pragma once
#include <pp_common/_.hpp>

bool OnDSDServerSidePacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
void OnDSDServerSideClose(const xTcpServiceClientConnectionHandle & Handle);

bool OnRegisterServiceProvider(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
