#pragma once
#include <pp_common/_.hpp>

class xPP_PostTcpData : public xBinaryMessage {
public:
    void SerializeMembers() override {
        assert(PayloadView.data() && PayloadView.size());
        W(DeviceSideConnectionId, RelaySideConnectionId, PayloadView);
    }
    void DeserializeMembers() override { R(DeviceSideConnectionId, RelaySideConnectionId, PayloadView); }

public:
    uint32_t         DeviceSideConnectionId;
    uint64_t         RelaySideConnectionId;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
};

class xPP_PostUdpData : public xBinaryMessage {
public:
    void SerializeMembers() override {
        assert(PayloadView.data() && PayloadView.size());
        W(DeviceSideConnectionId, RelaySideConnectionId, TargetAddress, PayloadView);
    }
    void DeserializeMembers() override { R(DeviceSideConnectionId, RelaySideConnectionId, TargetAddress, PayloadView); }

public:
    uint32_t         DeviceSideConnectionId;
    uint64_t         RelaySideConnectionId;
    xNetAddress      TargetAddress;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
};