#pragma once
#include <pp_common/base.hpp>

class xPR_Challenge : public xBinaryMessage {  // from proxy_access to relay server
public:
    void SerializeMembers() override {
        W(Timestamp, ChallengeKey, ChallengeHash);
    }
    void DeserializeMembers() override {
        R(Timestamp, ChallengeKey, ChallengeHash);
    }

public:
    uint64_t    Timestamp;
    std::string ChallengeKey;
    std::string ChallengeHash;
};

class xPR_ChallengeResp : public xBinaryMessage {  // from proxy_access to relay server
public:
    void SerializeMembers() override {
        W(Accepted);
    }
    void DeserializeMembers() override {
        R(Accepted);
    }

public:
    bool Accepted;
};