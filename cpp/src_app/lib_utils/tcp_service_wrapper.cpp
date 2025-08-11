#include "./tcp_service_wrapper.hpp"

void * xTcpServiceMessageChannel::GetUnderLayeredObject() const {
    auto Conn = Owner->GetConnection(ConnectionId);
    assert(!Conn || Conn == ConnectionPtr);
    return Conn;
}

void xTcpServiceMessageChannel::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const {
    Owner->PostMessage(ConnectionId, CmdId, RequestId, Message);
}

void xTcpServiceMessageChannel::PostMessageUnchecked(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const {
    Owner->PostMessage(*ConnectionPtr, CmdId, RequestId, Message);
}
