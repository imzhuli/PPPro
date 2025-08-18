#pragma once
#include <pp_common/_.hpp>

class xClientWrapper final : private xClient {

public:
    bool Init(xIoContext * ICP);
    void Clean();
    void UpdateTarget(const xNetAddress & Address);
    void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
    void Tick(uint64_t NowMS);
    void Kill();

    using xOnConnectedCallback    = std::function<void()>;
    using xOnDisconnectedCallback = std::function<void()>;
    using xOnPacketCallback       = std::function<bool(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

    xOnConnectedCallback    OnConnectedCallback;
    xOnDisconnectedCallback OnDisconnectedCallback;
    xOnPacketCallback       OnPacketCallback;

private:
    void OnServerConnected() override;
    void OnServerClose() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    bool         HasInstance = false;
    xIoContext * ICP         = nullptr;
};
