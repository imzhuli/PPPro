#pragma once
#include <pp_common/_.hpp>

struct xRL_RelayContextTimeoutNode : xListNode {
    uint64_t TimestampMS;
};

struct xRelayContextState {
    static constexpr const uint16_t TCP_CONNECTING  = 1;
    static constexpr const uint16_t TCP_ESTABLISHED = 2;
    static constexpr const uint16_t UDP_BINDING     = 3;
    static constexpr const uint16_t UDP_READY       = 4;
};

struct xRL_RelayContext : xRL_RelayContextTimeoutNode {
    uint64_t RelaySideContextId;
    uint64_t DeviceId;
    uint32_t DeviceSideContextId;
    uint16_t ContextState;
    uint64_t ProxyConnectionId;
    uint64_t ProxySideContextId;
};
using xRL_RelayContextTimeoutList = xList<xRL_RelayContext>;

extern void InitRelayContextPool();
extern void CleanRelayContextPool();

extern void               ReleaseRelayContext(xRL_RelayContext *);
extern xRL_RelayContext * GetRelayContextById(uint64_t RelaySideContextId);
extern void               KeepAlive(xRL_RelayContext *);
extern void               ReleaseTimeoutRelayContext();

extern xRL_RelayContext * CreateTcpConnection(uint64_t DeviceId, const xNetAddress & TargetAddress);
extern xRL_RelayContext * CreateTcpConnection(uint64_t DeviceId, const std::string_view & HostnameView, uint16_t Port);
extern xRL_RelayContext * CreateUdpChannel(uint64_t DeviceId);
extern xRL_RelayContext * CreateUdpChannel4(uint64_t DeviceId);
extern xRL_RelayContext * CreateUdpChannel6(uint64_t DeviceId);

extern void NotifyProxyConnectionRefused(xRL_RelayContext * PRC);
extern void NotifyProxyConnectionEstablished(xRL_RelayContext * PRC);

extern void PostConnectionData(uint64_t RelaySideContextId, const void * DataPtr, size_t DataSize);
extern void PostUdpChannelData(uint64_t RelaySideContextId, const xNetAddress & TargetAddress, const void * DataPtr, size_t DataSize);
extern void UdpKeepAlive(uint64_t RelaySideContextId);
