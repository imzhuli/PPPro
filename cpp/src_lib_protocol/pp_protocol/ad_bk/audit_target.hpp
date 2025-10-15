#pragma once
#include <pp_common/_.hpp>

struct xAD_BK_Target : xBinaryMessage {

    void SerializeMembers() override {
        W(LocalAuditTimestampMS);
        W(AuditId);
        W(TargetView);
        W(HitCount);
    }

    void DeserializeMembers() override {
        R(LocalAuditTimestampMS);
        R(AuditId);
        R(TargetView);
        R(HitCount);
    }

    uint64_t         LocalAuditTimestampMS;
    uint64_t         AuditId;
    std::string_view TargetView;
    uint64_t         HitCount;

    //
};
