#include "./register_server_client.hpp"

namespace {

    class xRegisterServerClientMessageChannel final : protected xMessageChannel {
    private:
        friend class ::xRegisterServerClient;
        xRegisterServerClient * Owner = nullptr;

        xRegisterServerClientMessageChannel(xRegisterServerClient * RSC) : Owner(RSC) {}
        void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override { Owner->PostMessage(CmdId, RequestId, Message); }
    };

}  // namespace

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
    if (LocalServerId) {
        ServerRegister(xRegisterServerClientMessageChannel(this), LocalServerId);
    }
}
