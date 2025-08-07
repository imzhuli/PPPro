#include "./udp_service_wrapper.hpp"

void xUdpServiceWrapper::OnPacket(const xNetAddress & RemoteAddress, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (OnPacketCallback) {
        OnPacketCallback(RemoteAddress, CommandId, RequestId, PayloadPtr, PayloadSize);
    }
}
