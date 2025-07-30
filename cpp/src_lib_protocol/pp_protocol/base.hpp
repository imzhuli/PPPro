#pragma once
#include <pp_common/_.hpp>

class xGroupedMessage : public xBinaryMessage {
public:
    void SerializeMembers() override { W(GroupId); }
    void DeserializeMembers() override { R(GroupId); }

public:
    xGroupId GroupId = 0;
};
