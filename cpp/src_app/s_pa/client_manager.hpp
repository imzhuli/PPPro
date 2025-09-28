#pragma once
#include <pp_common/_.hpp>

class xPA_ClientConnection;

enum xPA_ClientState {
    CS_CHALLENGE,  // challenge, protocol unknown
    CS_KILL_ON_FLUSH,

    CS_S5_CHALLENGE,                     // s5 challenge
    CS_S5_WAIT_FOR_AUTH_INFO,            // wait for auth info
    CS_S5_WAIT_FOR_IP_WHITELIST,         // no-auth but ip white list required
    CS_S5_WAIT_FOR_TARGET_ADDRESS,       //
    CS_S5_WAIT_FOR_CONECTION_ESTABLISH,  //
    CS_S_TC,                             // tcp connection

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

    // members:
    xPA_ClientTcpConnection Conn;
    xPA_ClientState         State = CS_CHALLENGE;
};

extern void InitClientManager();
extern void CleanClietManager();
extern void TickClientManager(uint64_t NowMS);

extern void DeferKillClientConnection(xPA_ClientConnection * ClientConnection);
extern void SchedulePassiveKillClientConnection(xPA_ClientConnection * ClientConnection);

extern void AsyncRequireAuthInfo(uint64_t ClientConnectionId, const std::string_view & AuthView);

// private functions:

extern xPA_ClientConnection * AcceptClientConnection(xSocket && NativeHandle);
extern void                   DestroyClientConnection(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionAccepted(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionPeerClose(xPA_ClientConnection * CC);
extern void                   OnPAClientConnectionFlush(xPA_ClientConnection * CC);
extern size_t                 OnPAClientConnectionData(xPA_ClientConnection * CC, ubyte * DP, size_t DS);

extern size_t OnPAC_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S5_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S5_AuthInfo(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
extern size_t OnPAC_S5_TargetAddress(xPA_ClientConnection * CC, ubyte * DP, size_t DS);
