#pragma once
#include <pp_common/_.hpp>

class xClientWrapper : private xClient {

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

    void SetOnConnectedCallback(const xOnConnectedCallback & CB) { OnConnectedCallback = CB; }
    void SetOnDisconnectedCallback(const xOnDisconnectedCallback & CB) { OnDisconnectedCallback = CB; }
    void SetOnPacketCallback(const xOnPacketCallback & CB) { OnPacketCallback = CB; }

private:
    void OnServerConnected() override;
    void OnServerClose() override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    bool         HasInstance = false;
    xIoContext * ICP         = nullptr;

    xOnConnectedCallback    OnConnectedCallback;
    xOnDisconnectedCallback OnDisconnectedCallback;
    xOnPacketCallback       OnPacketCallback;
};
