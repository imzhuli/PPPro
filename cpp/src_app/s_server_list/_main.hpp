#pragma once
#include "../lib_utils/all.hpp"
#include "./server_list_manager.hpp"

// clang-format off
extern void OnRegisterClientClose(const xTcpServiceClientConnectionHandle & Handle);
extern bool OnRegisterClientPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnRegisterAuthCacheServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnRegisterAuditAccountServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnRegisterAuditDeviceServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnRegisterAuditTargetServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnRegisterDeviceStateRelayServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnRegisterRelayInfoDispatcherServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnRegisterDeviceSelectorDispatcherServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);

extern bool OnDownloadClientPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadSelectorDispatcherList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadAuthCacheServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadAuditAccountServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadAuditDeviceServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadAuditTargetServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadDeviceStateRelayServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadBackendServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadRelayInfoDispatcherServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
extern bool OnDownloadDeviceSelectorDispatcherList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize);
// clang-format on
