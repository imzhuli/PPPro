#pragma once
#include <pp_common/_.hpp>

extern bool OnProducerClientPacket(
    const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
);
extern void OnProducerClientKeepAlive(const xTcpServiceClientConnectionHandle & Handle);
extern void OnProducerClientClose(const xTcpServiceClientConnectionHandle & Handle);
