#pragma once
#include <pp_common/_.hpp>

class xPP_CreateConnection : public xBinaryMessage {
public:
    void SerializeMembers() override { W(RelaySideContextId, TargetAddress); }
    void DeserializeMembers() override { R(RelaySideContextId, TargetAddress); }

public:
    uint64_t    RelaySideContextId;
    xNetAddress TargetAddress;
};

class xPP_CreateConnectionHost : public xBinaryMessage {
public:
    void SerializeMembers() override { W(RelaySideContextId, HostnameView, Port); }
    void DeserializeMembers() override { R(RelaySideContextId, HostnameView, Port); }

public:
    uint64_t         RelaySideContextId;
    std::string_view HostnameView;
    uint16_t         Port;
};

class xPP_CreateConnectionResp : public xBinaryMessage {
public:
    void SerializeMembers() override { W(DeviceSideContextId, RelaySideContextId, Connected); }
    void DeserializeMembers() override { R(DeviceSideContextId, RelaySideContextId, Connected); }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
    bool     Connected;
};

class xPP_DestroyConnection : public xBinaryMessage {
public:
    void SerializeMembers() override { W(DeviceSideContextId, RelaySideContextId); }
    void DeserializeMembers() override { R(DeviceSideContextId, RelaySideContextId); }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
};

class xPP_DeviceConnectionState : public xBinaryMessage {  // device->relay
public:
    void SerializeMembers() override {
        W(DeviceSideContextId, RelaySideContextId, ProxySideContextId);
        W(TotalBytesFromTarget, TotalBytesFromRelay);
    }
    void DeserializeMembers() override {
        R(DeviceSideContextId, RelaySideContextId, ProxySideContextId);
        R(TotalBytesFromTarget, TotalBytesFromRelay);
    }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
    uint64_t ProxySideContextId;
    uint64_t TotalBytesFromTarget = 0;
    uint64_t TotalBytesFromRelay  = 0;
};
