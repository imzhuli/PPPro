#pragma once
#include <pp_common/_.hpp>
#include <server_arch/client.hpp>

extern uint64_t LoadLocalServerId(const std::string & LocalServerIdFilename);
extern void     DumpLocalServerId(const std::string & LocalServerIdFilename, uint64_t LocalServerId);

class xServerIdClient final : public xClient {
public:
    bool Init(xIoContext * ICP, xNetAddress TargetAddress, uint64_t FirstTryServerId = 0);
    bool Init(xIoContext * ICP, xNetAddress TargetAddress, const std::string & LocalServerIdFilename);
    void Clean();
    using xClient::Tick;

    uint64_t GetLocalServerId() const { return LocalServerId; }

    using xOnServerIdUpdateCallback = std::function<void(uint64_t)>;
    xOnServerIdUpdateCallback OnServerIdUpdateCallback;

private:
    void OnServerConnected() final override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnServerIdUpdated(uint64_t NewServerId) { OnServerIdUpdateCallback(NewServerId); }

private:
    uint64_t LocalServerId = 0;
};
