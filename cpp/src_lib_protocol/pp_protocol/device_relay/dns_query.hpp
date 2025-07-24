#pragma once
#include <pp_common/base.hpp>

class xTR_DnsQuery : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(HostnameView);
    }
    void DeserializeMembers() override {
        R(HostnameView);
    }

public:
    std::string_view HostnameView;
};

class xTR_DnsQueryResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(HostnameView, PrimaryIpv4, PrimaryIpv6);
    }
    void DeserializeMembers() override {
        R(HostnameView, PrimaryIpv4, PrimaryIpv6);
    }

public:
    std::string_view HostnameView;
    xNetAddress      PrimaryIpv4;
    xNetAddress      PrimaryIpv6;
};
