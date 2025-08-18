#pragma once
#include "../base.hpp"

struct xCC_DeviceChallenge : xBinaryMessage {
    void SerializeMembers() override {
        W(AppVersion, ChannelId, Timestamp);
        W(PrimaryIpv4Address);
        W(PrimaryIpv6Address);
        W(Ipv4UdpEnabled);
        W(Ipv6UdpEnabled);
        W(Sign);
    };

    void DeserializeMembers() override {
        R(AppVersion, ChannelId, Timestamp);
        R(PrimaryIpv4Address);
        R(PrimaryIpv6Address);
        R(Ipv4UdpEnabled);
        R(Ipv6UdpEnabled);
        R(Sign);
    };

    uint32_t AppVersion = 0;
    uint32_t ChannelId  = 0;
    uint64_t Timestamp  = 0;

    xNetAddress PrimaryIpv4Address;
    xNetAddress PrimaryIpv6Address;
    bool        Ipv4UdpEnabled = false;
    bool        Ipv6UdpEnabled = false;

    std::string Sign;
};

struct xCC_DeviceChallengeResp : xBinaryMessage {
    void SerializeMembers() override { W(Address, CheckKey, TerminalAddress, UseOldVersion, EnableSpeedControl); };

    void DeserializeMembers() override { R(Address, CheckKey, TerminalAddress, UseOldVersion, EnableSpeedControl); };

    xNetAddress Address;
    std::string CheckKey;

    xNetAddress TerminalAddress;
    bool        UseOldVersion      = false;
    bool        EnableSpeedControl = false;
    //
};
