#pragma once
#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>
#include <unordered_map>

class xDS_DeviceSelectorServiceProvider {

public:
    bool Init(xIoContext * ICP);
    void Tick(uint64_t NowMS);
    void Clean();

    void OnClientConnected(const xMessageChannel & Source);
    bool OnClientPacket(const xMessageChannel & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

    void UpdateDispatcherList(const std::vector<xDeviceSelectorDispatcherInfo> & ServerList);

protected:
    void RegisterServiceProvider(const xMessageChannel & Poster);
    bool OnRegisterServerResp(ubyte * PayloadPtr, size_t PayloadSize);
    bool OnSelectDevice(const xMessageChannel & Source, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
    xClientPoolWrapper ClientPool;
};
