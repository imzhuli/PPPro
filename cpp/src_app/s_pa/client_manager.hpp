#pragma once
#include "../lib_client/device_selector_client.hpp"

#include <pp_common/_.hpp>

struct xPA_ClientConnection;

enum xPA_ClientState {
    CS_CHALLENGE,  // challenge, protocol unknown
    CS_KILL_ON_FLUSH,

    CS_S5_CHALLENGE,                     // s5 challenge
    CS_S5_WAIT_FOR_AUTH_INFO,            // wait for auth info
    CS_S5_WAIT_FOR_AUTH_RESULT,          //
    CS_S5_WAIT_FOR_DEVICE_RESULT,        //
    CS_S5_WAIT_FOR_TARGET_ADDRESS,       //
    CS_S5_WAIT_FOR_CONECTION_ESTABLISH,  //
    CS_S5_READY,                         //

    CS_H_CHALLENGE,                     // http proxy
    CS_H_WAIT_FOR_AUTH_RESULT,          //
    CS_H_WAIT_FOR_DEVICE_RESULT,        //
    CS_H_WAIT_FOR_CONECTION_ESTABLISH,  //
    CS_H_READY,

    CS_T_CHALLENGE,                     // http tunnel (CONNECT) proxy
    CS_T_WAIT_FOR_AUTH_RESULT,          //
    CS_T_WAIT_FOR_DEVICE_RESULT,        //
    CS_T_WAIT_FOR_CONECTION_ESTABLISH,  //
    CS_T_READY,

    CS_U_HOLDING,  // udp bound channel holder
};

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

    // members:
    xPA_ClientTcpConnection Conn;
    xPA_ClientState         State = CS_CHALLENGE;
    uint64_t                ConnectionId;

    uint64_t DeviceRelayServerRuntimeId = 0;
    uint64_t DeviceRelaySideId          = 0;
    uint64_t RelaySideContextId         = 0;

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

// private functions:

extern xPA_ClientConnection * AcceptClientConnection(xSocket && NativeHandle);
extern void                   DestroyClientConnection(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionAccepted(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionPeerClose(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionFlush(xPA_ClientConnection * CC);
extern size_t                 OnPAClientConnectionData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);

extern void OnPAC_AuthResult(uint64_t ConnectionId, const xClientAuthResult * AR);
extern void OnPAC_DeviceSelectResult(uint64_t ConnectionId, const xDeviceSelectorResult & Result);

extern size_t OnPAC_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);

// s5-client
extern size_t OnPAC_S5_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S5_AuthInfo(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S5_TargetAddress(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S5_UploadData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
// s5-internal
extern void OnPAC_S5_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_S5_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result);
extern void OnPAC_S5_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId);

// hn-client
extern size_t OnPAC_H_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_H_UploadData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
// hn-internal
extern void OnPAC_H_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_H_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result);
extern void OnPAC_H_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId);

// ht-client
extern size_t OnPAC_T_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_T_UploadData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
// ht-internal
extern void OnPAC_T_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR);
extern void OnPAC_T_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result);
extern void OnPAC_T_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId);

static constexpr auto HTTP_404 = "HTTP/1.1 407 Not Found\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_407 = "HTTP/1.1 407 Proxy Authentication Required\r\nProxy-Authenticate: Basic realm=Restricted\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_500 = "HTTP/1.1 500 Internal server error\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_502 = "HTTP/1.1 502 Target Unreached\r\nConnection: close\r\n\r\n"sv;
static constexpr auto HTTP_200 = "HTTP/1.1 200 Connection established\r\nProxy-agent: proxy / 1.0\r\n\r\n"sv;
