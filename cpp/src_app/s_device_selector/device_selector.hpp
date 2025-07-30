#pragma once
#include "../lib_server_util/all.hpp"

#include <pp_common/_.hpp>
#include <unordered_map>

class xDS_DeviceSelectorServiceProvider {

public:
    bool Init(xIoContext * ICP);
    void Tick(uint64_t NowMS);
    void Clean();

    // void OnClientConnected(xServiceClientConnection & Connection) override;
    // void OnClientClose(xServiceClientConnection & Connection) override;
    // bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    // void OnCleanupClientConnection(const xServiceClientConnection & Connection) override;

    void UpdateDispatcherList(const std::vector<xDeviceSelectorDispatcherInfo> & ServerList);

protected:
    bool OnSelectDevice(xServiceClientConnection & CC, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
    xClientPoolWrapper ClientPool;
};

struct xDS_DeviceObserver : public xClientPool {
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
};
