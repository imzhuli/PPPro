#pragma once
#include <pp_common/base.hpp>

class xTR_PostData : public xBinaryMessage {
public:
    void SerializeMembers() override {
        assert(PayloadView.data() && PayloadView.size());
        W(DeviceSideConnectionId, RelaySideConnectionId, PayloadView);
    }
    void DeserializeMembers() override {
        R(DeviceSideConnectionId, RelaySideConnectionId, PayloadView);
    }

public:
    uint32_t         DeviceSideConnectionId;
    uint64_t         RelaySideConnectionId;
    std::string_view PayloadView;

    static constexpr const size_t MAX_PAYLOAD_SIZE = 4096;
    static_assert(MAX_PAYLOAD_SIZE <= MaxPacketPayloadSize - 32);
};
