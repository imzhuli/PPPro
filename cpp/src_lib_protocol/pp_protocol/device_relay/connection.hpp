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

class xTR_ConnectionStateNotify : public xBinaryMessage {
public:
    static constexpr uint32_t STATE_UNSEPC          = 0;
    static constexpr uint32_t STATE_ESTABLISHED     = 1;
    static constexpr uint32_t STATE_UPDATE_TRANSFER = 2;
    static constexpr uint32_t STATE_CLOSED          = 3;

    static const char * GetStateName(int State) {
        switch (State) {
            case STATE_UNSEPC:
                return "STATE_UNSEPC";
            case STATE_ESTABLISHED:
                return "STATE_ESTABLISHED";
            case STATE_UPDATE_TRANSFER:
                return "STATE_UPDATE_TRANSFER";
            case STATE_CLOSED:
                return "STATE_CLOSED";
            default:
                break;
        }
        return "INVALID_CONNECTION_STATE";
    }

public:
    void SerializeMembers() override {
        W(DeviceSideContextId, RelaySideContextId);
        W(NewState, TotalReadBytes, TotalWrittenBytes);
    }
    void DeserializeMembers() override {
        R(DeviceSideContextId, RelaySideContextId);
        R(NewState, TotalReadBytes, TotalWrittenBytes);
    }

public:
    uint32_t DeviceSideContextId;
    uint64_t RelaySideContextId;
    uint32_t NewState          = 0;
    uint64_t TotalReadBytes    = 0;
    uint64_t TotalWrittenBytes = 0;
};
