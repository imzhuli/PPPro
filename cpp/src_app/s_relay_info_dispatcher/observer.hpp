#pragma once
#include <pp_common/_.hpp>

extern void BroadCastRelayInfo(const xRelayServerInfoBase & Info);
extern void OnObserverClientConnected(const xTcpServiceClientConnectionHandle & Handle);
extern void OnObserverClientClose(const xTcpServiceClientConnectionHandle & Handle);
