#pragma once
#include <pp_common/_.hpp>

class xInitDataStream : public xBinaryMessage {
public:
    void SerializeMembers() override {
        auto KeyOpt = PackDataKey();
        if (!KeyOpt) {
            GetWriter()->SetError();
        }
        W(CtrlId, *KeyOpt, DeviceLocalIdString);
    }
    void DeserializeMembers() override {
        std::string Key;
        R(CtrlId, Key, DeviceLocalIdString);
        if (!UnpackDataKey(Key)) {
            GetReader()->SetError();
        }
    }

    auto PackDataKey() -> xOptional<std::string>;
    bool UnpackDataKey(const std::string & Key);

public:
    uint64_t    CtrlId;
    std::string DeviceLocalIdString;

    //
    xNetAddress InternalIpv4Address;
    xNetAddress InternalIpv6Address;
    std::string DataKey;
};

class xInitDataStreamResp : public xBinaryMessage {
public:
    void SerializeMembers() override { W(Accepted); }
    void DeserializeMembers() override { R(Accepted); }

public:
    bool Accepted;
};
