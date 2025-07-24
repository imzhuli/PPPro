#pragma once
#include <pp_common/base.hpp>

class xTR_CreateUdpChannel : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(RelaySideChannelId);
    }
    void DeserializeMembers() override {
        R(RelaySideChannelId);
    }

public:
    uint64_t RelaySideChannelId;
};

class xTR_CreateUdpChannelResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(DeviceSideChannelId, RelaySideChannelId);
    }
    void DeserializeMembers() override {
        R(DeviceSideChannelId, RelaySideChannelId);
    }

public:
    uint32_t DeviceSideChannelId;
    uint64_t RelaySideChannelId;
};

class xTR_UdpChannelKeepAlive : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(DeviceSideChannelId, RelaySideChannelId);
    }
    void DeserializeMembers() override {
        R(DeviceSideChannelId, RelaySideChannelId);
    }

public:
    uint32_t DeviceSideChannelId;
    uint64_t RelaySideChannelId;
};

class xTR_UdpChannelClose : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(DeviceSideChannelId, RelaySideChannelId);
    }
    void DeserializeMembers() override {
        R(DeviceSideChannelId, RelaySideChannelId);
    }

public:
    uint32_t DeviceSideChannelId;
    uint64_t RelaySideChannelId;
};

class xTR_PostUdpData : public xBinaryMessage {
public:
    void SerializeMembers() override {
        assert(PayloadView.data() && PayloadView.size());
        W(DeviceSideChannelId, RelaySideChannelId, PayloadView);
    }
    void DeserializeMembers() override {
        R(DeviceSideChannelId, RelaySideChannelId, PayloadView);
    }

public:
    uint32_t         DeviceSideChannelId;
    uint64_t         RelaySideChannelId;
    std::string_view PayloadView;

    static constexpr const size32_t MAX_PAYLOAD_SIZE = 4096;
    static_assert(MAX_PAYLOAD_SIZE <= MaxPacketPayloadSize - 32);
};
