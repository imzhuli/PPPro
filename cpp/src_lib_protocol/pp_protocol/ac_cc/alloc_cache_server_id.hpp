#pragma once
#include "../base.hpp"

struct x_AC_CC_AllocServerId : xBinaryMessage {

    void SerializeMembers() override;
    void DeserializeMembers() override;

    uint64_t TimestampMS;
    //
};

struct x_AC_CC_AllocServerIdResp : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
    }
    void DeserializeMembers() override {
        R(ServerId);
    }
    uint32_t ServerId;
    //
};
