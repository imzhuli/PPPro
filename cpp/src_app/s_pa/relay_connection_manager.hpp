#pragma once
#include "../lib_utils/all.hpp"

void InitRelayConnectionManager();
void CleanRelayConnectionManager();
void TickRelayConnectionManager(uint64_t NowMS);

bool PostRelayMessage(uint64_t RelayServerId, xPacketCommandId CmdId, xPacketRequestId RequestId, xel::xBinaryMessage & Message);
bool RequestRelayTargetConnection(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t DeviceRelaySideId, const xNetAddress & TargetAddress);
bool RequestRelayTargetConnection(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t DeviceRelaySideId, const std::string_view & TargetHost, uint16_t TargetPort);
bool RequestRelayPostConnectionData(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t RelaySideContextId, const void * DP, size_t DS);
void RequestRelayCloseConnection(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t RelaySideContextId);

bool RequestRelayUdpBinding(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t DeviceRelaySideId);
void RequestRelayKeepAliveUdpBinding(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t RelaySideContextId);
void RequestRelayPostUdpData(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t RelaySideContextId, const xNetAddress TargetAddress, const void * DP, size_t DS);
void RequestRelayCloseUdpBinding();
