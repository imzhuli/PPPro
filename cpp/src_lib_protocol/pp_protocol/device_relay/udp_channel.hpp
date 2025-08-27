#pragma once
#include <pp_common/_.hpp>

class xPP_CreateUdpChannel : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(RelaySideContextId);
        W(Enable4, Enable6);
    }
    void DeserializeMembers() override {
        R(RelaySideContextId);
        R(Enable4, Enable6);
    }

public:
    uint64_t RelaySideContextId;
    bool     Enable4;
    bool     Enable6;
};

class xPP_CreateUdpChannelResp : public xBinaryMessage {
public:
    void SerializeMembers() override { W(DeviceSideContextId, RelaySideContextId); }
    void DeserializeMembers() override { R(DeviceSideContextId, RelaySideContextId); }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
};

class xPP_UdpChannelKeepAlive : public xBinaryMessage {
public:
    void SerializeMembers() override { W(DeviceSideContextId, RelaySideContextId); }
    void DeserializeMembers() override { R(DeviceSideContextId, RelaySideContextId); }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
};

class xPP_DestroyUdpChannel : public xBinaryMessage {
public:
    void SerializeMembers() override { W(DeviceSideContextId, RelaySideContextId); }
    void DeserializeMembers() override { R(DeviceSideContextId, RelaySideContextId); }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
};
