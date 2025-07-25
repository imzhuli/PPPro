#pragma once
#include "../base.hpp"

struct xPP_QueryAuthCache : xBinaryMessage {

    void SerializeMembers() override { W(UserPass); }
    void DeserializeMembers() override { R(UserPass); }

    std::string_view UserPass;
    //
};

struct xPP_QueryAuthCacheResp : xBinaryMessage {

    void SerializeMembers() override {
        W(Result.AuditId);
        W(Result.CountryId);
        W(Result.StateId);
        W(Result.CityId);
        W(Result.RequireIpv6);
        W(Result.RequireUdp);
        W(Result.AutoChangeIp);
        W(Result.PAToken);
    }
    void DeserializeMembers() override {
        R(Result.AuditId);
        R(Result.CountryId);
        R(Result.StateId);
        R(Result.CityId);
        R(Result.RequireIpv6);
        R(Result.RequireUdp);
        R(Result.AutoChangeIp);
        R(Result.PAToken);
    }

    xClientAuthResult Result;
};
