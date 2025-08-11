#include "./register_server_client.hpp"

void xRegisterServerClientMessageChannel::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const {
    Owner->PostMessage(CmdId, RequestId, Message);
}

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

void xRegisterServerClient::OnServerConnected() {
    if (LocalServerId && IdPoster) {
        IdPoster(xRegisterServerClientMessageChannel(this), LocalServerId);
    }
}
