#include "./client_pool_wrapper.hpp"

bool xClientPoolWrapper::Init(xIoContext * ICP, size_t MaxConnectionCount) {
    if (!xClientPool::Init(ICP, MaxConnectionCount)) {
        return false;
    }
    xClientPool::OnServerConnected = std::bind(&xClientPoolWrapper::OnServerConnectedCallback, this, std::placeholders::_1);
    xClientPool::OnServerClose     = std::bind(&xClientPoolWrapper::OnServerCloseCallback, this, std::placeholders::_1);
    xClientPool::OnServerPacket    = std::bind(
        &xClientPoolWrapper::OnServerPacketCallback,  //
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5
    );
    return true;
}

void xClientPoolWrapper::Clean() {
    xClientPool::Clean();
    Reset(SortedServerList);
}

void xClientPoolWrapper::AddServer(const xNetAddress & Address) {
    auto Temp = xCPW_InternalServerInfo{
        Address,
        0,
    };
    auto LB    = std::lower_bound(SortedServerList.begin(), SortedServerList.end(), Temp);
    auto Found = (LB != SortedServerList.end() && *LB == Temp);
    if (Found) {
        return;
    }

    auto NewId = xClientPool::AddServer(Address);
    if (!NewId) {
        return;
    }
    Temp.LocalServerIndex = NewId;
    SortedServerList.insert(LB, Temp);
}

void xClientPoolWrapper::RemoveServer(const xNetAddress & Address) {
    auto Temp = xCPW_InternalServerInfo{
        Address,
        0,
    };
    auto LB    = std::lower_bound(SortedServerList.begin(), SortedServerList.end(), Temp);
    auto Found = (LB != SortedServerList.end() && *LB == Temp);
    if (!Found) {
        return;
    }
    xClientPool::RemoveServer(LB->LocalServerIndex);
    SortedServerList.erase(LB);
}

void xClientPoolWrapper::UpdateServerList(const std::vector<xNetAddress> & ServerAddressList) {
    auto Add = std::vector<xNetAddress>();
    auto Rem = std::vector<xNetAddress>();

    size_t IOld = 0;
    size_t INew = 0;
    while (IOld < SortedServerList.size() && INew < ServerAddressList.size()) {
        auto & O = SortedServerList[IOld];
        auto & N = ServerAddressList[INew];
        if (O.Address < N) {
            Rem.push_back(O.Address);
            ++IOld;
            continue;
        }
        if (N < O.Address) {
            Add.push_back(N);
            ++INew;
            continue;
        }
        ++IOld;
        ++INew;
    }
    for (; INew < ServerAddressList.size(); ++INew) {
        Add.push_back(ServerAddressList[INew]);
    }
    for (; IOld < SortedServerList.size(); ++IOld) {
        Rem.push_back(SortedServerList[IOld].Address);
    }
    for (const auto & A : Add) {
        AddServer(A);
    }
    for (const auto & R : Rem) {
        RemoveServer(R);
    }

    OnUpdateServerListCallback(Add, Rem);
}

void xClientPoolWrapper::PostMessageByConnectionId(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    xClientPool::PostMessage(ConnectionId, CmdId, RequestId, Message);
}

void xClientPoolWrapper::PostMessageByHash(uint64_t Hash, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    if (SortedServerList.empty()) {
        return;
    }
    auto   Index     = Hash % SortedServerList.size();
    auto & ServerRef = SortedServerList[Index];
    xClientPool::PostMessage(ServerRef.LocalServerIndex, CmdId, RequestId, Message);
}

void xClientPoolWrapper::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    xClientPool::PostMessage(CmdId, RequestId, Message);
}

void xClientPoolWrapper::OnServerConnectedCallback(xClientConnection & CC) {
    auto Poster = xCPW_MessageChannel{
        this,
        &CC,
    };
    OnConnectedCallback(Poster);
}

void xClientPoolWrapper::OnServerCloseCallback(xClientConnection & CC) {
    auto Poster = xCPW_MessageChannel{
        this,
        &CC,
    };
    OnDisonnectedCallback(Poster);
}

bool xClientPoolWrapper::OnServerPacketCallback(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Poster = xCPW_MessageChannel{
        this,
        &CC,
    };
    return OnPacketCallback(Poster, CommandId, RequestId, PayloadPtr, PayloadSize);
}
