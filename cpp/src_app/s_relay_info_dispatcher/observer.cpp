#include "./global.hpp"

struct xRID_ObserverConnectionNode : xListNode {
    xRID_ObserverConnectionNode(const xTcpServiceClientConnectionHandle & Handle) : Handle(Handle) {}

    xTcpServiceClientConnectionHandle Handle;
};
using xRID_ObserverConnectionList = xList<xRID_ObserverConnectionNode>;

static xRID_ObserverConnectionList ObserverConnectionList;

void BroadCastRelayInfo(const xRelayServerInfoBase & Info) {
    DEBUG_LOG();

    auto Msg       = xPP_BroadcastRelayInfo();
    Msg.ServerInfo = Info;
    ubyte B[MaxPacketSize];
    auto  RS = WriteMessage(B, Cmd_BroadcastRelayInfo, 0, Msg);

    ObserverConnectionList.ForEach([&B, &RS](auto & N) { N.Handle.PostData(B, RS); });
}

void OnObserverClientConnected(const xTcpServiceClientConnectionHandle & Handle) {
    DEBUG_LOG("HandleId: %" PRIx64 "", Handle.GetConnectionId());
    auto NP = new xRID_ObserverConnectionNode(Handle);
    ObserverConnectionList.AddTail(*NP);
    Handle->UserContext.P = NP;
}

void OnObserverClientClose(const xTcpServiceClientConnectionHandle & Handle) {
    DEBUG_LOG("HandleId: %" PRIx64 "", Handle.GetConnectionId());
    auto NP = static_cast<xRID_ObserverConnectionNode *>(Steal(Handle->UserContext.P));
    delete NP;
}
