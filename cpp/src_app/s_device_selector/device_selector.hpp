#pragma once
#include <pp_common/base.hpp>
#include <unordered_map>

struct xDS_DeviceSelectorService : public xService {

    void OnClientConnected(xServiceClientConnection & Connection) override;
    void OnClientClose(xServiceClientConnection & Connection) override;
    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
    void OnCleanupClientConnection(const xServiceClientConnection & Connection) override;

protected:
    bool OnSelectDevice(xServiceClientConnection & CC, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
};

struct xDS_DeviceObserver : public xClientPool {
    bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;
};
