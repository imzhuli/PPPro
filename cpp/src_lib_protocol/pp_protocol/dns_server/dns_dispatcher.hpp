#pragma once
#include "../base.hpp"

class xCheckinDnsServer : public xBinaryMessage {
public:
	void SerializeMembers() override;
	void DeserializeMembers() override;

	xGroupId    GroupId;
	uint64_t    UnixTimestamp = 0;
	std::string VersionKey;
};

class xCheckinDnsServerResp : public xBinaryMessage {
public:
	void SerializeMembers() override {
		W(Accepted);
	};

	void DeserializeMembers() override {
		R(Accepted);
	};

	bool Accepted;
};
