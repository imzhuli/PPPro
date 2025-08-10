#include "./tcp_service_wrapper.hpp"

void xTcpServiceMessagePoster::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const {
    Owner->PostMessage(ConnectionId, CmdId, RequestId, Message);
}

void xTcpServiceMessagePoster::PostMessageUnchecked(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const {
    Owner->PostMessage(*ConnectionPtr, CmdId, RequestId, Message);
}
