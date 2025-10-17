#pragma once
#include <pp_common/_.hpp>

class xPR_CreateConnection : public xBinaryMessage {
public:
    void SerializeMembers() override { W(RelayServerSideDeviceId, ProxySideContextId, TargetAddress, HostnameView, HostnamePort); }
    void DeserializeMembers() override { R(RelayServerSideDeviceId, ProxySideContextId, TargetAddress, HostnameView, HostnamePort); }

public:
    uint64_t         RelayServerSideDeviceId;
    uint64_t         ProxySideContextId;
    xNetAddress      TargetAddress;
    std::string_view HostnameView;
    uint16_t         HostnamePort;
};

class xPR_DestroyConnection : public xBinaryMessage {
public:
    void SerializeMembers() override { W(ProxySideContextId, RelaySideContextId); }
    void DeserializeMembers() override { R(ProxySideContextId, RelaySideContextId); }

public:
    uint64_t ProxySideContextId;
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

class xPR_PushTcpData : public xBinaryMessage {
public:
    void SerializeMembers() override { W(ProxySideContextId, RelaySideContextId, PayloadView); }
    void DeserializeMembers() override { R(ProxySideContextId, RelaySideContextId, PayloadView); }

    uint64_t         ProxySideContextId;
    uint64_t         RelaySideContextId;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
    static_assert(MAX_PAYLOAD_SIZE <= MaxPacketPayloadSize - 32);
};

struct xPR_CreateUdpBinding : xBinaryMessage {

    void SerializeMembers() override { W(RelayServerSideDeviceId, ProxySideContextId); }
    void DeserializeMembers() override { R(RelayServerSideDeviceId, ProxySideContextId); }

    uint64_t RelayServerSideDeviceId;
    uint64_t ProxySideContextId;

    //
};

struct xPR_KeepAliveUdpBinding : xBinaryMessage {

    void SerializeMembers() override { W(ProxySideContextId, RelaySideContextId); }
    void DeserializeMembers() override { R(ProxySideContextId, RelaySideContextId); }

    uint64_t ProxySideContextId;
    uint64_t RelaySideContextId;

    //
};

struct xPR_DestroyUdpBinding : xBinaryMessage {

    void SerializeMembers() override { W(ProxySideContextId, RelaySideContextId); }
    void DeserializeMembers() override { R(ProxySideContextId, RelaySideContextId); }

    uint64_t ProxySideContextId;
    uint64_t RelaySideContextId;
};

struct xPR_PushUdpData : xBinaryMessage {

public:
    void SerializeMembers() override { W(ProxySideContextId, RelaySideContextId, TargetAddress, PayloadView); }
    void DeserializeMembers() override { R(ProxySideContextId, RelaySideContextId, TargetAddress, PayloadView); }

    uint64_t         ProxySideContextId;
    uint64_t         RelaySideContextId;
    xNetAddress      TargetAddress;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
    static_assert(MAX_PAYLOAD_SIZE <= MaxPacketPayloadSize - 128);
};
