#pragma once
#include <pp_common/_.hpp>
#include <pp_common/region.hpp>
#include <server_arch/client_pool.hpp>

class xClientPoolWrapper final : private xClientPool {

public:
    bool Init(xIoContext * ICP, size_t MaxConnectionCount = 1024);
    void Clean();
    using xClientPool::Tick;

    void AddServer(const xNetAddress & Address);
    void RemoveServer(const xNetAddress & Address);
    void UpdateServerList(const std::vector<xNetAddress> & ServerAddressList);

    void PostMessageByConnectionId(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
    void PostMessageByHash(uint64_t Hash, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
    void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

    using xOnUpdateServerListCallback = std::function<void(const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed)>;
    using xOnConnectedCallback        = std::function<void(const xMessageChannel & Source)>;
    using xOnDisonnectedCallback      = std::function<void(const xMessageChannel & Source)>;
    using xOnPacketCallback = std::function<bool(const xMessageChannel & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

    xOnUpdateServerListCallback OnUpdateServerListCallback = Noop<>;
    xOnConnectedCallback        OnConnectedCallback        = Noop<>;
    xOnDisonnectedCallback      OnDisonnectedCallback      = Noop<>;
    xOnPacketCallback           OnPacketCallback           = Noop<true>;

private:
    void OnServerConnectedCallback(xClientConnection & CC);
    void OnServerCloseCallback(xClientConnection & CC);
    bool OnServerPacketCallback(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
    struct xCPW_InternalServerInfo final {
        xNetAddress Address;
        xIndexId    LocalServerIndex;

        auto operator<=>(const xCPW_InternalServerInfo & O) const { return Address <=> O.Address; }
        auto operator==(const xCPW_InternalServerInfo & O) const { return Address == O.Address; }
    };

    class xCPW_MessageChannel final : public xMessageChannel {
    public:
        xCPW_MessageChannel(xClientPool * CPP, xClientConnection * CCP) {
            this->CPP          = CPP;
            this->CCP          = CCP;
            this->ConnectionId = CCP->GetConnectionId();
        }

        void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override {  //
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
};
