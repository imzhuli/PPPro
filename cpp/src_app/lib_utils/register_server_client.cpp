#include "./register_server_client.hpp"

void xRegisterServerClient::Clean() {
    Reset(LocalServerId);
    xClient::Clean();
}

void xRegisterServerClient::SetLocalServerId(uint64_t NewServerId) {
    LocalServerId = NewServerId;
    if (IsOpen()) {
        Kill();
    }
}

void xRegisterServerClient::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    xClient::PostMessage(CmdId, RequestId, Message);
}

void xRegisterServerClient::OnServerConnected() {
    if (LocalServerId && IdPoster) {
        IdPoster(this, LocalServerId);
    }
}
