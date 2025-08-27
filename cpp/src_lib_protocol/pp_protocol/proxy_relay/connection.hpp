#pragma once
#include <pp_common/_.hpp>

class xPR_CreateConnection : public xBinaryMessage {
public:
    void SerializeMembers() override { W(RelaySideDeviceId, ProxySideConnectionId, TargetAddress, HostnameView, HostnamePort); }
    void DeserializeMembers() override { R(RelaySideDeviceId, ProxySideConnectionId, TargetAddress, HostnameView, HostnamePort); }

public:
    uint64_t         RelaySideDeviceId;
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

class xPR_ConnectionStateNotify : public xBinaryMessage {
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
        W(ProxySideConnectionId, RelaySideContextId);
        W(NewState);
        W(TotalUploadedBytes, TotalDumpedBytes);
    }
    void DeserializeMembers() override {
        R(ProxySideConnectionId, RelaySideContextId);
        R(NewState);
        R(TotalUploadedBytes, TotalDumpedBytes);
    }

    std::string ToString() const;

public:
    uint64_t ProxySideConnectionId;
    uint64_t RelaySideContextId;
    uint32_t NewState;
    uint64_t TotalUploadedBytes = 0;
    uint64_t TotalDumpedBytes   = 0;
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
