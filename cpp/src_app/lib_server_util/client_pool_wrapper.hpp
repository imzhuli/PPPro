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
    void PostMessage(uint64_t Hash, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
    void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

    using xOnUpdateServerListCallback = std::function<void(const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed)>;
    void SetOnUpdateServerListCallback(const xOnUpdateServerListCallback & Callback) { OnUpdateServerListCallback = Callback; }

    using xOnPacketCallback = std::function<bool(xMessagePoster * Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;
    void SetOnPacketCallback(const xOnPacketCallback & CB) { OnPacketCallback = CB; }

private:
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
    struct xCPW_InternalServerInfo final {
        xNetAddress Address;
        xIndexId    LocalServerIndex;

        auto operator<=>(const xCPW_InternalServerInfo & O) const { return Address <=> O.Address; }
        auto operator==(const xCPW_InternalServerInfo & O) const { return Address == O.Address; }
    };

    struct xCPW_MessagePoster final : xMessagePoster {
    public:
        xCPW_MessagePoster(xClientPool * CPP, xClientConnection * CCP) {
            this->CPP = CPP;
            this->CCP = CCP;
        }
        void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) override { CPP->PostMessage(*CCP, CmdId, RequestId, Message); }

    private:
        xClientPool *       CPP;
        xClientConnection * CCP;
    };

    std::vector<xCPW_InternalServerInfo> SortedServerList;
    xOnUpdateServerListCallback          OnUpdateServerListCallback;
    xOnPacketCallback                    OnPacketCallback;
};
