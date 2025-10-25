#pragma once
#include "../lib_client/device_selector_client.hpp"

#include <pp_common/_.hpp>

static constexpr const uint64_t MAX_CLIENT_CONNECTION_AUTH_TIMEOUT_MS         = 3'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_PASSIVE_KILL_TIMEOUT_MS = 10'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_IDLE_TIMEOUT_MS         = 90'000;
static constexpr const uint64_t MAX_CLIENT_CONNECTION_ID_COUNT                = 250'000;

static constexpr auto HTTP_404 = "HTTP/1.1 407 Not Found\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_407 = "HTTP/1.1 407 Proxy Authentication Required\r\nProxy-Authenticate: Basic realm=Restricted\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_500 = "HTTP/1.1 500 Internal server error\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_502 = "HTTP/1.1 502 Target Unreached\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_200 = "HTTP/1.1 200 Connection established\r\nProxy-agent: proxy / 1.0\r\n\r\n"sv;

enum struct ePA_ClientMainState : uint8_t {
    C = 0,  // challenge: init status
    F = 1,  // Final / flush to kill
    S = 2,  // s5
    H = 3,  // HTTP NORMAL
    T = 4,  // HTTP TUNNEL
};

enum struct ePA_ClientSubState : uint8_t {
    _ = 0,
    WAIT_FOR_AUTH_INFO,                 // wait for auth info
    WAIT_FOR_AUTH_RESULT,               // wait for auth result
    LOCK_AND_GET_DEVICE_CACHE,          // device cache
    WAIT_FOR_DEVICE_SELECTION,          // device selection
    WAIT_FOR_TARGET,                    //
    WAIT_FOR_TCP_CONNECTION_ESTABLISH,  //
    WAIT_FOR_UDP_BINDING,
};

struct xPA_ClientState {
    ePA_ClientMainState M = ePA_ClientMainState::C;
    ePA_ClientSubState  S = ePA_ClientSubState::_;
    bool                A = false;  // AuthInfoEnabled
};

struct xPA_ClientConnection;
struct xPA_ClientTcpConnection;

struct xPA_ClientTcpConnection : xTcpConnection {
    xPA_ClientTcpConnection(xPA_ClientConnection * Owner) : Owner(Owner){};
    xPA_ClientConnection * Owner;
};

struct xPA_ClientConnectionIdleNode : xListNode {
    uint64_t LastActivityTimestampMS = 0;
};
struct xPA_ClientConnectionKillNode : xListNode {
    uint64_t KillingScheduledTimestampMS = 0;
};

struct xPA_ClientConnection
    : xPA_ClientConnectionIdleNode
    , xPA_ClientConnectionKillNode {

    xPA_ClientConnection() : Conn(this) {}

    xNetAddress GetRemoteAddress() const { return Conn.GetRemoteAddress(); }
    void        PostData(const void * DataPtr, size_t DataSize) { Conn.PostData(DataPtr, DataSize); }
    bool        HasPendingWrites() const { return Conn.HasPendingWrites(); }

    void SuspendReading() { Conn.SuspendReading(); }
    void ResumeReading() { Conn.ResumeReading(); }

    //
    xPA_ClientTcpConnection Conn;
    uint64_t                ConnectionId;
    xPA_ClientState         State;

    uint64_t DeviceRelayServerRuntimeId = 0;
    uint64_t DeviceRelaySideId          = 0;
    uint64_t RelaySideContextId         = 0;
    uint64_t UdpChannelId               = 0;

    struct {
        std::string RequestHeader;
        std::string TargetHost;
        uint16_t    TargetPort;
    } Http;
};

extern void InitClientManager();
extern void CleanClietManager();
extern void TickClientManager(uint64_t NowMS);

extern xPA_ClientConnection * GetClientConnectionById(uint64_t ClientConnectionId);
extern void                   KeepAlive(xPA_ClientConnection * ClientConnection);
extern void                   DeferKillClientConnection(xPA_ClientConnection * ClientConnection);
extern void                   SchedulePassiveKillClientConnection(xPA_ClientConnection * ClientConnection);
extern std::string            BuildIpAuthInfo(xPA_ClientConnection * ClientConnection);

extern xPA_ClientConnection * AcceptClientConnection(xSocket && NativeHandle);
extern void                   DestroyClientConnection(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionAccepted(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionPeerClose(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionFlush(xPA_ClientConnection * CC);
extern size_t                 OnPAClientConnectionData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);

////////// events and callbacks

extern size_t OnPAC_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern void   OnPAC_AuthResult(uint64_t ConnectionId, const xClientAuthResult * AR);
extern void   OnPAC_DeviceSelectResult(uint64_t ConnectionId, const xDeviceSelectorResult & Result);

// s5-client
extern size_t OnPAC_S_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S_AuthInfo(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S_TargetAddress(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S_UploadTcpData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
// s5-internal
extern void OnPAC_S_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_S_IpResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_S_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result);
extern void OnPAC_S_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId);
extern void OnPAC_S_UploadUdpData(xPA_ClientConnection * CC, const xNetAddress TargetAddress, ubyte * DP, size_t DS);

// hn-client
extern size_t OnPAC_H_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_H_UploadData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
// hn-internal
extern void OnPAC_H_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_H_IpResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_H_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result);
extern void OnPAC_H_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId);

// ht-client
extern size_t OnPAC_T_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_T_UploadData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
// ht-internal
extern void OnPAC_T_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_T_IpResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_T_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result);
extern void OnPAC_T_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId);
