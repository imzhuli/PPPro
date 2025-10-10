#pragma once
#include <pp_common/_.hpp>

void InitProxyService();
void CleanProxyService();
void TickProxyService(uint64_t NowMS);

bool OnProxyPacket(const xTcpServiceClientConnectionHandle &, xPacketCommandId, xPacketRequestId, ubyte *, size_t);

void PostDataToProxy(uint64_t ProxyConnectionId, const void * PL, size_t PS);
void PostMessageToProxy(uint64_t ProxyConnectionId, xPacketCommandId CmdId, xPacketRequestId ReqId, xel::xBinaryMessage & Msg);
