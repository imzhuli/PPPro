#pragma once
#include "../lib_utils/all.hpp"

void InitRelayConnectionManager();
void CleanRelayConnectionManager();
void TickRelayConnectionManager(uint64_t NowMS);

void UpdateRelayConnection(uint64_t RelayServerId, const xNetAddress & Address);
void RemoveRelayConnection(uint64_t RelayServerId);

void RequestRelayTargetConnection(uint64_t RelayServerId, uint64_t DeviceRelaySideId, const xNetAddress & TargetAddress);
void RequestRelayTargetConnection(uint64_t RelayServerId, uint64_t DeviceRelaySideId, const std::string_view & TargetHost);
