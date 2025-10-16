#pragma once
#include <pp_common/_.hpp>

struct xPP_AuditTarget : xBinaryMessage {

    void SerializeMembers() override {
        W(AuditId);
        W(TargetView);
        W(HitCount);
    }

    void DeserializeMembers() override {
        R(AuditId);
        R(TargetView);
        R(HitCount);
    }

    uint64_t         AuditId;
    std::string_view TargetView;
    uint64_t         HitCount;

    //
};
