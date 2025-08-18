#include "./server_id_client.hpp"

#include <fstream>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/server_id.hpp>

uint64_t LoadLocalServerId(const std::string & LocalServerIdFilename) {
    auto File  = LocalServerIdFilename;
    auto FSOpt = FileToStr(File);
    if (!FSOpt) {
        return 0;
    }
    return (uint64_t)strtoumax(FSOpt->c_str(), nullptr, 10);
}

void DumpLocalServerId(const std::string & LocalServerIdFilename, uint64_t LocalServerId) {
    if (LocalServerIdFilename.empty()) {
        return;
    }
    auto File  = LocalServerIdFilename;
    auto FSOpt = std::ofstream(File, std::ios_base::binary | std::ios_base::out);
    if (!FSOpt) {
        cerr << "failed to dump file to LocalCacheFile" << endl;
        return;
    }
    FSOpt << LocalServerId << endl;
    return;
}

//////////////////////////////////////////////////////

bool xServerIdClient::Init(xIoContext * ICP, xNetAddress TargetAddress, uint64_t FirstTryServerId) {
    if (!xClient::Init(ICP, TargetAddress)) {
        return false;
    }
    LocalServerId = FirstTryServerId;
    return true;
}

bool xServerIdClient::Init(xIoContext * ICP, xNetAddress TargetAddress, const std::string & LocalServerIdFilename) {
    if (!xClient::Init(ICP, TargetAddress)) {
        return false;
    }
    LocalServerId = LoadLocalServerId(LocalServerIdFilename);
    return true;
}

void xServerIdClient::Clean() {
    Reset(LocalServerId);
    xClient::Clean();
};

void xServerIdClient::OnServerConnected() {
    auto Req             = xPP_AcquireServerId();
    Req.PreviousServerId = LocalServerId;
    PostMessage(Cmd_AcquireServerId, 0, Req);
}

bool xServerIdClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    if (CommandId != Cmd_AcquireServerIdResp) {
        return false;
    }

    auto Resp = xPP_AcquireServerIdResp();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        X_DEBUG_PRINTF("invalid packet");
        return false;
    }
    LocalServerId = Resp.NewServerId;
    OnServerIdUpdated(LocalServerId);
    return true;
}
