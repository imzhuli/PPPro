#pragma once
#include "../lib_utils/all.hpp"

void InitRelayConnectionManager();
void CleanRelayConnectionManager();
void TickRelayConnectionManager(uint64_t NowMS);

bool PostRelayMessage(uint64_t RelayServerId, xPacketCommandId CmdId, xPacketRequestId RequestId, xel::xBinaryMessage & Message);
bool RequestRelayTargetConnection(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t DeviceRelaySideId, const xNetAddress & TargetAddress);
bool RequestRelayTargetConnection(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t DeviceRelaySideId, const std::string_view & TargetHost, uint16_t TargetPort);
void RequestRelayCloseConnection(uint64_t ProxyConnectionId, uint64_t RelayServerId, uint64_t RelaySideContextId);
