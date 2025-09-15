#pragma once
#include <pp_common/_.hpp>

class xPR_CreateConnection : public xBinaryMessage {
public:
    void SerializeMembers() override { W(RelayServerSideDeviceId, ProxySideConnectionId, TargetAddress, HostnameView, HostnamePort); }
    void DeserializeMembers() override { R(RelayServerSideDeviceId, ProxySideConnectionId, TargetAddress, HostnameView, HostnamePort); }

public:
    uint64_t         RelayServerSideDeviceId;
    uint64_t         ProxySideConnectionId;
    xNetAddress      TargetAddress;
    std::string_view HostnameView;
    uint16_t         HostnamePort;
};

class xPR_DestroyConnection : public xBinaryMessage {
public:
    void SerializeMembers() override { W(ProxySideConnectionId, RelaySideContextId); }
    void DeserializeMembers() override { R(ProxySideConnectionId, RelaySideContextId); }

public:
    uint64_t ProxySideConnectionId;
    uint64_t RelaySideContextId;
};

class xPP_ProxyConnectionState : public xBinaryMessage {  // device->relay
public:
    void SerializeMembers() override {
        W(DeviceSideContextId, RelaySideContextId, ProxySideContextId);
        W(TotalBytesFromRelay, TotalBytesFromClient);
    }
    void DeserializeMembers() override {
        R(DeviceSideContextId, RelaySideContextId, ProxySideContextId);
        R(TotalBytesFromRelay, TotalBytesFromClient);
    }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
    uint64_t ProxySideContextId;
    uint64_t TotalBytesFromRelay  = 0;
    uint64_t TotalBytesFromClient = 0;
};

class xPR_PushData : public xBinaryMessage {
public:
    void SerializeMembers() override { W(ProxySideConnectionId, RelaySideContextId, PayloadView); }
    void DeserializeMembers() override { R(ProxySideConnectionId, RelaySideContextId, PayloadView); }

public:
    uint64_t         ProxySideConnectionId;
    uint64_t         RelaySideContextId;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
    static_assert(MAX_PAYLOAD_SIZE <= MaxPacketPayloadSize - 32);
};
