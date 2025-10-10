#pragma once
#include <pp_common/_.hpp>

void InitProxyService();
void CleanProxyService();
void TickProxyService(uint64_t NowMS);

bool OnProxyPacket(const xTcpServiceClientConnectionHandle &, xPacketCommandId, xPacketRequestId, ubyte *, size_t);