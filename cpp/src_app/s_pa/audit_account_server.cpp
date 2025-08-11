#include "./audit_account_server.hpp"

#include "../lib_utils/all.hpp"

bool xPA_AuditAccountLocalServer::Init(xIoContext * ICP) {
    assert(ICP);
    if (!ClientHashPool.Init(ICP)) {
        return false;
    }
    ClientHashPool.SetOnPacketCallback(
        [this](const xMessageChannel & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) -> bool {
            return OnServerPacket(CommandId, RequestId, PayloadPtr, PayloadSize);
        }
    );

    return true;
}

void xPA_AuditAccountLocalServer::Clean() {
    ClientHashPool.Clean();
}

void xPA_AuditAccountLocalServer::Tick(uint64_t NowMS) {
    ClientHashPool.Tick(NowMS);
}

void xPA_AuditAccountLocalServer::UpdateServerList(const std::vector<xServerInfo> & ServerList) {
    auto AddressList = std::vector<xNetAddress>();
    for (auto S : ServerList) {
        AddressList.push_back(S.Address);
    }
    ClientHashPool.UpdateServerList(AddressList);
}

void xPA_AuditAccountLocalServer::CollectAuditAccountInfo(const xAuditAccountInfo & Info) {
    auto R                   = xAuditAccountUsage();
    R.AuditId                = Info.AuditId;
    R.UploadSize             = Info.TotalUploadSize;
    R.DownloadSize           = Info.TotalDownloadSize;
    R.TcpIncreament          = Info.TotalTcpCount;
    R.UdpIncreament          = Info.TotalUdpCount;
    R.DeviceChangeIncreament = 0;
    ClientHashPool.PostMessageByHash(Info.AuditId, Cmd_AuditAccountUsage, 0, R);
}

bool xPA_AuditAccountLocalServer::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    DEBUG_LOG("CommondId=%" PRIx32 ", RequestId:%" PRIx64 " Data=\n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
    return true;
}
