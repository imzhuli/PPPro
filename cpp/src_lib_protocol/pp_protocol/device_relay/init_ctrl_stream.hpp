#pragma once
#include <pp_common/base.hpp>

class xInitCtrlStream : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(CheckKey, Ipv4Address, Ipv6Address, Resolved3rdIpv6);
    }
    void DeserializeMembers() override {
        R(CheckKey, Ipv4Address, Ipv6Address, Resolved3rdIpv6);
    }

public:
    std::string CheckKey;
    xNetAddress Ipv4Address;
    xNetAddress Ipv6Address;
    bool        Resolved3rdIpv6;
};

class xInitCtrlStreamResp : public xBinaryMessage {
public:
    void SerializeMembers() override {
        W(DeviceId, CtrlId, DeviceKey, EnableIpv6);
    }
    void DeserializeMembers() override {
        R(DeviceId, CtrlId, DeviceKey, EnableIpv6);
    }

public:
    uint64_t    DeviceId;
    uint64_t    CtrlId;
    std::string DeviceKey;
    bool        EnableIpv6;
};
