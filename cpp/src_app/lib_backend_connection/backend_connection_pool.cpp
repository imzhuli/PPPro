#include "./backend_connection_pool.hpp"

#include <pp_protocol/_backend/backend_challenge.hpp>
#include <pp_protocol/command.hpp>

bool xBackendConnectionPool::Init(xIoContext * ICP, size_t MaxConnectionCount) {
    if (!xClientPool::Init(ICP, MaxConnectionCount)) {
        return false;
    }
    ContextList.resize(MaxConnectionCount);
    return true;
}

void xBackendConnectionPool::Clean() {
    xClientPool::Clean();
    Reset(ContextList);
    ResetLocalAudit();
}

uint64_t xBackendConnectionPool::AddServer(const xNetAddress & Address, const std::string & AppKey, const std::string & AppSecret) {
    auto CompareTemp = xBackendServerInfo{ Address, 0 };
    auto Iter        = std::lower_bound(SortedServerList.begin(), SortedServerList.end(), CompareTemp);
    if (Iter != SortedServerList.end() && *Iter == CompareTemp) {  // found
        ++TotalAddingServerConflict;
        return 0;
    }
    auto Sid = xClientPool::AddServer(Address);
    if (!Sid) {
        return 0;
    }

    auto Index = Sid.GetIndex();
    assert(Index < ContextList.size());
    auto & Ctx = ContextList[Index];

    Ctx.AppKey    = AppKey;
    Ctx.AppSecret = AppSecret;

    CompareTemp.ConnectionId = Sid;
    SortedServerList.insert(Iter, CompareTemp);

    ++TotalAddedServer;
    return Sid;
}

void xBackendConnectionPool::RemoveServer(const xNetAddress & Address) {
    auto LI = std::lower_bound(SortedServerList.begin(), SortedServerList.end(), xBackendServerInfo{ Address });
    if (LI == SortedServerList.end() || LI->Address != Address) {
        ++TotalRemovingServerFailure;
        return;
    }
    auto ConnectionId = LI->ConnectionId;

    auto Sid = xIndexId(ConnectionId);
    Reset(ContextList[Sid.GetIndex()]);
    xClientPool::RemoveServer(ConnectionId);

    SortedServerList.erase(LI);
    ++TotalRemovedServer;
}

void xBackendConnectionPool::OnServerConnected(xClientConnection & CC) {
    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];

    auto challenge           = xBackendChallenge();
    challenge.AppKey         = Ctx.AppKey;
    challenge.TimestampMS    = xel::GetTimestampMS();
    challenge.ChallengeValue = challenge.GenerateChallengeString(Ctx.AppSecret);
    ubyte Buffer[xel::MaxPacketSize];
    auto  RSize = xel::WriteMessage(Buffer, Cmd_BackendChallenge, 0, challenge);
    CC.PostData(Buffer, RSize);

    X_DEBUG_PRINTF("Sending:\n%s", HexShow(Buffer, RSize).c_str());
    X_DEBUG_PRINTF("Header: %s", StrToHex(Buffer, PacketHeaderSize).c_str());
    X_DEBUG_PRINTF("Body: %s", StrToHex(Buffer + PacketHeaderSize, RSize - PacketHeaderSize).c_str());
}

bool xBackendConnectionPool::OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId == Cmd_BackendChallengeResp) {
        return OnCmdBackendChallengeResp(CC, CommandId, RequestId, PayloadPtr, PayloadSize);
    }

    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];
    if (!Ctx.IsChallengeReady) {
        return false;
    }

    if (BackendPacketCallback) {
        BackendPacketCallback(CommandId, RequestId, PayloadPtr, PayloadSize);
    }
    return true;
}

void xBackendConnectionPool::OnServerClose(xClientConnection & CC) {
    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];
    Reset(Ctx);
}

bool xBackendConnectionPool::OnCmdBackendChallengeResp(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto   Sid = CC.GetConnectionId();
    auto   Idx = Sid.GetIndex();
    auto & Ctx = ContextList[Idx];

    if (Ctx.IsChallengeReady) {
        X_DEBUG_PRINTF("invalid challenge state");
        return false;
    }

    auto R = xBackendChallengeResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("failed server challenge");
        return false;
    }

    X_DEBUG_PRINTF("server challenge ready");
    Ctx.IsChallengeReady = true;
    return true;
}

std::string xBackendConnectionPool::GetLocalAudit() {
    auto OS = std::ostringstream();
    OS << "TotalAddedServer: " << TotalAddedServer << endl;
    OS << "TotalRemovedServer: " << TotalRemovedServer << endl;
    OS << "TotalRemovingServerFailure: " << TotalRemovingServerFailure;
    return OS.str();
}

void xBackendConnectionPool::ResetLocalAudit() {
    Reset(TotalAddedServer);
    Reset(TotalRemovedServer);
    Reset(TotalAddingServerConflict);
    Reset(TotalRemovingServerFailure);
}
