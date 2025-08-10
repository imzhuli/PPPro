#pragma once
#include <pp_common/_.hpp>

class xUdpServiceWrapper final : private xel::xUdpService {

public:
    using xUdpService::Init;
    //
    using xUdpService::Clean;
    //
    using xUdpService::PostMessage;

    using xOnPacketCallback =
        std::function<void(const xNetAddress & RemoteAddress, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

    void SetOnPacketCallback(const xOnPacketCallback & CB) { OnPacketCallback = CB; }

private:
    void OnPacket(const xNetAddress & RemoteAddress, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    xOnPacketCallback OnPacketCallback;
};
