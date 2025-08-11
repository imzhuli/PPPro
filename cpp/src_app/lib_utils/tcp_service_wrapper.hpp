#pragma once
#include <pp_common/_.hpp>

class xTcpServiceWrapper;

class xTcpServiceMessageChannel final : public xMessageChannel {

public:
    xTcpServiceMessageChannel(xTcpServiceWrapper & Service, xServiceClientConnection & SCC) {
        Owner         = &Service;
        ConnectionPtr = &SCC;
        ConnectionId  = SCC.GetConnectionId();
        assert(ConnectionId);
    }

    void * GetUnderLayeredObject() const override;
    void * GetUnderLayeredObjectUnchecked() const override { return ConnectionPtr; }
    void   PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override;
    void   PostMessageUnchecked(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override;

private:
    friend class xTcpServiceWrapper;

private:
    xTcpServiceWrapper *       Owner         = nullptr;
    xServiceClientConnection * ConnectionPtr = nullptr;
    uint64_t                   ConnectionId  = 0;
};

class xTcpServiceWrapper final : protected xService {

public:
    using xService::Init;
    //
    using xService::Clean;
    //
    using xService::Tick;
    //
    using xService::PostMessage;
    //
    using xService::DeferKillConnection;

    using xOnClientConnectedCallback = std::function<void(const xTcpServiceMessageChannel & Channel)>;
    using xOnClientPacketCallback =
        std::function<bool(const xTcpServiceMessageChannel & Channel, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;
    using xOnClientCloseCallback = std::function<void(const xTcpServiceMessageChannel & Channel)>;

    void SetOnClientConnectedCallback(const xOnClientConnectedCallback & CB) { OnClientConnectedCallback = CB; }
    void SetOnClientPacketCallback(const xOnClientPacketCallback & CB) { OnClientPacketCallback = CB; }
    void SetOnClientCloseCallback(const xOnClientCloseCallback & CB) { OnClientCloseCallback = CB; }

    xTcpServiceMessageChannel Wrap(xServiceClientConnection & ServiceClientConnection) {
        assert(GetConnection(ServiceClientConnection.GetConnectionId()) == &ServiceClientConnection);
        return { *this, ServiceClientConnection };
    }

private:
    void OnClientConnected(xServiceClientConnection & Connection) override { OnClientConnectedCallback(Wrap(Connection)); }
    void OnClientClose(xServiceClientConnection & Connection) override { OnClientCloseCallback(Wrap(Connection)); }
    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        return OnClientPacketCallback(Wrap(Connection), CommandId, RequestId, PayloadPtr, PayloadSize);
    }

    friend class xTcpServiceMessageChannel;

    xOnClientConnectedCallback OnClientConnectedCallback = IgnoreConnetionEvent;
    xOnClientPacketCallback    OnClientPacketCallback    = IgnorePacket;
    xOnClientCloseCallback     OnClientCloseCallback     = IgnoreConnetionEvent;

    static void IgnoreConnetionEvent(const xTcpServiceMessageChannel & Channel) {}
    static bool IgnorePacket(const xTcpServiceMessageChannel &, xPacketCommandId, xPacketRequestId, ubyte *, size_t) { return true; }

    //
};
