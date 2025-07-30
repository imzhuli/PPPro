#pragma once
#include <pp_common/_.hpp>

struct xInitDataKeyPacker : public xBinaryMessage {

    void SerializeMembers() override { W(InternalIpv4Address, InternalIpv6Address, CheckKey); }
    void DeserializeMembers() override { R(InternalIpv4Address, InternalIpv6Address, CheckKey); }

    xNetAddress InternalIpv4Address;
    xNetAddress InternalIpv6Address;
    std::string CheckKey;

    //
};

extern std::string MakeInternalKey(const xNetAddress & InternalIpv4Address, const xNetAddress & InternalIpv6Address, const std::string & CheckKey);
