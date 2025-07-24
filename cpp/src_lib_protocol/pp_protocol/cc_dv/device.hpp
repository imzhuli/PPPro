#pragma once
#include "../base.hpp"

struct xCC_DeviceChallenge : xBinaryMessage {
    void SerializeMembers() override {
        W(AppVersion, Timestamp, Sign);
    };

    void DeserializeMembers() override {
        R(AppVersion, Timestamp, Sign);
    };

    uint32_t    AppVersion = 0;
    uint64_t    Timestamp  = 0;
    std::string Sign;
};

struct xCC_DeviceChallengeResp : xBinaryMessage {
    void SerializeMembers() override {
        W(CtrlAddress, DataAddress, CheckKey, TerminalAddress, UseOldVersion, EnableSpeedControl);
    };

    void DeserializeMembers() override {
        R(CtrlAddress, DataAddress, CheckKey, TerminalAddress, UseOldVersion, EnableSpeedControl);
    };

    xNetAddress CtrlAddress;
    xNetAddress DataAddress;
    std::string CheckKey;

    xNetAddress TerminalAddress;
    bool        UseOldVersion      = false;
    bool        EnableSpeedControl = false;
    //
};
