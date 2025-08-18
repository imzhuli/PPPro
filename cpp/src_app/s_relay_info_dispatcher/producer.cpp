#include "./global.hpp"

bool OnProducerClientPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    auto & P = Handle->UserContext.P;
    if (P) {
        DEBUG_LOG("duplicate server registration");
        return false;
    }

    if (CommandId != Cmd_RelayServerHeartBeat) {
        DEBUG_LOG("invalid command id");
        return false;
    }

    auto R = xPP_RelayServerHeartBeat();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol base");
        return false;
    }

    auto InfoCopy = new xRelayServerInfoBase(R.ServerInfo);
    P             = InfoCopy;
    ++LocalAudit.TotalRegisteredConnections;
    ++LocalAudit.TotalAvailableRelayConnections;

    Logger->I("New relay server info: %s", InfoCopy->ToString().c_str());
    BroadCastRelayInfo(*InfoCopy);

    return true;
}

void OnProducerClientKeepAlive(const xTcpServiceClientConnectionHandle & Handle) {
    auto InfoCopy = static_cast<xRelayServerInfoBase *>(Handle->UserContext.P);
    if (!InfoCopy) {
        return;
    }
    BroadCastRelayInfo(*InfoCopy);
}

void OnProducerClientClose(const xTcpServiceClientConnectionHandle & Handle) {
    ++LocalAudit.TotalClosedConnections;
    auto InfoCopy = static_cast<xRelayServerInfoBase *>(Steal(Handle->UserContext.P));
    if (!InfoCopy) {
        return;
    }
    delete InfoCopy;
    --LocalAudit.TotalAvailableRelayConnections;
}
