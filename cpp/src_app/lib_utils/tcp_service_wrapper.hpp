#pragma once
#include <pp_common/_.hpp>

class xTcpServiceMessagePoster;
class xTcpServiceWrapper;

class xTcpServiceMessagePoster final : public xMessagePoster {

public:
    xTcpServiceMessagePoster(xTcpServiceWrapper & Service, xServiceClientConnection & SCC) {
        Owner         = &Service;
        ConnectionPtr = &SCC;
        ConnectionId  = SCC.GetConnectionId();
        assert(ConnectionId);
    }

    uint64_t GetInternalId() const override { return ConnectionId; }
    void     PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override;
    void     PostMessageUnchecked(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override;

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

    using xOnClientConnectedCallback = std::function<void(xServiceClientConnection & Connection)>;
    using xOnClientCloseCallback     = std::function<void(xServiceClientConnection & Connection)>;
    using xOnClientPacketCallback =
        std::function<bool(const xMessagePoster & Poster, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

private:
    xOnClientConnectedCallback OnClientConnectedCallback = IgnoreConnetionEvent;
    xOnClientCloseCallback     OnClientCloseCallback     = IgnoreConnetionEvent;
    xOnClientPacketCallback    OnClientPacketCallback    = IgnorePacket;

    static void IgnoreConnetionEvent(xServiceClientConnection & Connection) {}
    static bool IgnorePacket(const xMessagePoster & Poster, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) { return true; }

    //
};
