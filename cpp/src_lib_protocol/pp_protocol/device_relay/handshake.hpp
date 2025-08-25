#pragma once
#include <pp_common/_.hpp>

class xPP_DeviceHandshake : public xBinaryMessage {
public:
    void SerializeMembers() override { W(HandshakeKey); }
    void DeserializeMembers() override { R(HandshakeKey); }

public:
    std::string HandshakeKey;
};

class xPP_DeviceHandshakeResp : public xBinaryMessage {
public:
    void SerializeMembers() override { W(Accepted); }
    void DeserializeMembers() override { R(Accepted); }

public:
    bool Accepted;
};
