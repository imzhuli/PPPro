#pragma once
#include <pp_common/_.hpp>

class xPP_PostConnectionData : public xBinaryMessage {
public:
    void SerializeMembers() override {
        assert(PayloadView.data() && PayloadView.size());
        W(DeviceSideContextId, RelaySideContextId, PayloadView);
    }
    void DeserializeMembers() override { R(DeviceSideContextId, RelaySideContextId, PayloadView); }

public:
    uint32_t         DeviceSideContextId;
    uint64_t         RelaySideContextId;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
};

class xPP_PostUdpChannelData : public xBinaryMessage {
public:
    void SerializeMembers() override {
        assert(PayloadView.data() && PayloadView.size());
        W(DeviceSideContextId, RelaySideContextId, TargetAddress, PayloadView);
    }
    void DeserializeMembers() override { R(DeviceSideContextId, RelaySideContextId, TargetAddress, PayloadView); }

public:
    uint32_t         DeviceSideContextId;
    uint64_t         RelaySideContextId;
    xNetAddress      TargetAddress;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
};