#pragma once
#include <pp_common/_.hpp>
#include <server_arch/client.hpp>

extern uint64_t LoadLocalServerId(const std::string & LocalServerIdFilename);
extern void     DumpLocalServerId(const std::string & LocalServerIdFilename, uint64_t LocalServerId);

class xServerIdClient final : public xel::xClient {
public:
    bool Init(xIoContext * ICP, xNetAddress TargetAddress, uint64_t FirstTryServerId = 0) {
        if (!xClient::Init(ICP, TargetAddress)) {
            return false;
        }
        LocalServerId = FirstTryServerId;
        return true;
    }
    bool Init(xIoContext * ICP, xNetAddress TargetAddress, const std::string & LocalServerIdFilename) {
        if (!xClient::Init(ICP, TargetAddress)) {
            return false;
        }
        LocalServerId = LoadLocalServerId(LocalServerIdFilename);
        return true;
    }
    void Clean() {
        Reset(LocalServerId);
        xClient::Clean();
    };
    using xClient::Tick;

    void     SetOnServerIdUpdateCallback(const std::function<void(uint64_t)> & C) { OnServerIdUpdateCallback = C; }
    uint64_t GetLocalServerId() const { return LocalServerId; }

protected:
    virtual void OnServerIdUpdated(uint64_t NewServerId) { OnServerIdUpdateCallback(NewServerId); }

private:
    void OnServerConnected() final override;
    bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) final override;

private:
    uint64_t                      LocalServerId            = 0;
    std::function<void(uint64_t)> OnServerIdUpdateCallback = {};
};
