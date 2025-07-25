#include "./_global.hpp"

#define DECLARE_STATE_HANDLE(type, name)     \
    static type              name##Instance; \
    xPA_ClientStateHandler * name = &name##Instance

DECLARE_STATE_HANDLE(xPA_ClientStateHandler_CloseStateHandler, CloseClientStateHandler);
DECLARE_STATE_HANDLE(xPA_ClientStateHandler_TestProxyType, TestProxyTypetState);
DECLARE_STATE_HANDLE(xPA_ClientStateHandler_S5_Challenge, S5ChallengeStateHandler);
DECLARE_STATE_HANDLE(xPA_ClientStateHandler_S5_WaitForAuthInfo, S5WaitForAuthInfoStateHandler);
DECLARE_STATE_HANDLE(xPA_ClientStateHandler_S5_WaitForAuthResult, S5WaitForAuthResultStateHandler);
DECLARE_STATE_HANDLE(xPA_ClientStateHandler_Http_Challenge, HttpChallengeStateHandler);

//////////////////

xPA_ClientConnection * UpCast(xTcpConnection * CP) {
    return static_cast<xPA_ClientConnection *>(CP);
}

void ScheduleClientStateChange(xPA_ClientConnection * Client, xPA_ClientStateHandler * NextStateHandler) {
    assert(!Client->StateContext.NextHandler);
    Client->StateContext.NextHandler = NextStateHandler;
}

void InitClientState(xPA_ClientConnection * Client) {
    assert(!Client->StateContext.CurrentHandler);
    assert(!Client->StateContext.NextHandler);
    TestProxyTypetState->OnEntry(Client);
    assert(!Client->StateContext.CurrentHandler);
    assert(!Client->StateContext.NextHandler);  // no state change during OnExit callback
    Client->StateContext.CurrentHandler = TestProxyTypetState;
}

bool UpdateClientState(xPA_ClientConnection * Client) {
    if (auto N = Steal(Client->StateContext.NextHandler)) {
        Steal(Client->StateContext.CurrentHandler)->OnExit(Client);  // prevent double entry
        assert(!Client->StateContext.CurrentHandler);
        assert(!Client->StateContext.NextHandler);  // no state change during OnExit callback
        N->OnEntry(Client);
        assert(!Client->StateContext.CurrentHandler);
        assert(!Client->StateContext.NextHandler);  // no state change during OnEntry callback
        Client->StateContext.CurrentHandler = N;
        return true;
    }
    return false;
}

void FinalClientState(xPA_ClientConnection * Client) {
    Steal(Client->StateContext.CurrentHandler)->OnExit(Client);
}

void KillConnection(xPA_ClientConnection * CP) {
    assert(CP && CP->Owner);
    CP->Owner->KillConnection(CP);
}
