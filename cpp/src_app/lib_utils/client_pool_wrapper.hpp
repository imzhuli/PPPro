#pragma once
#include <pp_common/_.hpp>
#include <pp_common/region.hpp>
#include <server_arch/client_pool.hpp>

class xClientPoolWrapper final : private xClientPool {

public:
    using xClientPool::Init;
    using xClientPool::Tick;
    void Clean();

    void AddServer(const xNetAddress & Address);
    void RemoveServer(const xNetAddress & Address);
    void UpdateServerList(const std::vector<xNetAddress> & ServerAddressList);

    void PostMessageByConnectionId(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
    void PostMessageByHash(uint64_t Hash, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
    void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

    using xOnUpdateServerListCallback = std::function<void(const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed)>;
    void SetOnUpdateServerListCallback(const xOnUpdateServerListCallback & Callback) { OnUpdateServerListCallback = Callback; }

    using xOnConnectedCallback = std::function<void(const xMessagePoster & Source)>;
    void SetOnConnectedCallback(const xOnConnectedCallback & CB) { OnConnectedCallback = CB; }

    using xOnPacketCallback = std::function<bool(const xMessagePoster & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;
    void SetOnPacketCallback(const xOnPacketCallback & CB) { OnPacketCallback = CB; }

private:
    void OnServerConnected(xClientConnection & CC) override;
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    struct xCPW_InternalServerInfo final {
        xNetAddress Address;
        xIndexId    LocalServerIndex;

        auto operator<=>(const xCPW_InternalServerInfo & O) const { return Address <=> O.Address; }
        auto operator==(const xCPW_InternalServerInfo & O) const { return Address == O.Address; }
    };

    class xCPW_MessagePoster final : public xMessagePoster {
    public:
        xCPW_MessagePoster(xClientPool * CPP, xClientConnection * CCP) {
            this->CPP          = CPP;
            this->CCP          = CCP;
            this->ConnectionId = CCP->GetConnectionId();
        }

        uint64_t GetInternalId() const override { return ConnectionId; }
        void     PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override {  //
            CPP->PostMessage(ConnectionId, CmdId, RequestId, Message);
        }
        void PostMessageUnchecked(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override {
            CPP->PostMessage(*CCP, CmdId, RequestId, Message);
        }

    private:
        xClientPool *       CPP;
        xClientConnection * CCP;
        uint64_t            ConnectionId;
    };

    std::vector<xCPW_InternalServerInfo> SortedServerList;
    xOnUpdateServerListCallback          OnUpdateServerListCallback = IgnoreUpdateServerList;
    xOnConnectedCallback                 OnConnectedCallback        = IgnoreConnected;
    xOnPacketCallback                    OnPacketCallback           = IgnorePacket;

private:
    static void IgnoreUpdateServerList(const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed) {}
    static void IgnoreConnected(const xMessagePoster & Source) {}
    static bool IgnorePacket(const xMessagePoster & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) { return true; }
};
