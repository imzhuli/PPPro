#include "./client_wrapper.hpp"

bool xClientWrapper::Init(xIoContext * ICP) {
    this->ICP = ICP;
    return true;
}

void xClientWrapper::Clean() {
    if (Steal(HasInstance)) {
        xClient::Clean();
    }
    Reset(ICP);
    Reset(OnConnectedCallback);
    Reset(OnDisconnectedCallback);
    Reset(OnPacketCallback);
}

void xClientWrapper::Tick(uint64_t NowMS) {
    if (!HasInstance) {
        return;
    }
    xClient::Tick(NowMS);
}

void xClientWrapper::Kill() {
    if (!HasInstance) {
        return;
    }
    xClient::Kill();
}

void xClientWrapper::UpdateTarget(const xNetAddress & Address) {
    if (Steal(HasInstance)) {
        xClient::Clean();
    }
    if (Address) {
        RuntimeAssert((HasInstance = xClient::Init(this->ICP, Address)));
    }
}

void xClientWrapper::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    if (!HasInstance) {
        return;
    }
    xClient::PostMessage(CmdId, RequestId, Message);
}

void xClientWrapper::OnServerConnected() {
    assert(HasInstance);
    if (OnConnectedCallback) {
        OnConnectedCallback();
    }
}

void xClientWrapper::OnServerClose() {
    assert(HasInstance);
    if (OnDisconnectedCallback) {
        OnDisconnectedCallback();
    }
}

bool xClientWrapper::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    assert(HasInstance);
    if (OnPacketCallback) {
        return OnPacketCallback(CommandId, RequestId, PayloadPtr, PayloadSize);
    }
    return true;
}
