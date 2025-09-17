#pragma once
#include <pp_common/_.hpp>

bool OnDSDClientSidePacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
bool OnDeviceSelect(const xTcpServiceClientConnectionHandle & Handle, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
