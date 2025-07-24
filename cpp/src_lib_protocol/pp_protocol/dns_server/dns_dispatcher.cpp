#include "./dns_dispatcher.hpp"

#include <crypto/md5.hpp>
#include <crypto/sha.hpp>

static const char * HashSaltPrefix   = "DnsDDDDDDispatcher99635";
static uint64_t     MaxTimestampDiff = 10 * 60'000;  // 10min

void xCheckinDnsServer::SerializeMembers() {
	auto Timestamp = UnixTimestamp;
	if (!Timestamp) {
		Timestamp = xel::GetUnixTimestamp();
	}
	auto HashSource = HashSaltPrefix + std::to_string(Timestamp) + VersionKey;
	auto R1         = Sha256(HashSource.data(), HashSource.size());
	auto R2         = Md5(R1.Data(), R1.Size());
	auto HR         = StrToHex(R2.Data(), R2.Size());
	W(GroupId, Timestamp, VersionKey, HR);
}

void xCheckinDnsServer::DeserializeMembers() {
	auto Reader = GetReader();
	assert(Reader);

	std::string HR;
	R(GroupId, UnixTimestamp, VersionKey, HR);
	if (Reader->HasError()) {
		return;
	}

	uint64_t Now  = GetUnixTimestamp();
	auto     Diff = xel::SignedDiff(Now, UnixTimestamp);

	X_DEBUG_PRINTF("CheckinDiff: %i", (int)Diff);
	if (abs(Diff) > MaxTimestampDiff) {  // too large timediff
		Reader->SetError();
		return;
	}

	auto HashSource = HashSaltPrefix + std::to_string(UnixTimestamp) + VersionKey;
	auto R1         = Sha256(HashSource.data(), HashSource.size());
	auto R2         = Md5(R1.Data(), R1.Size());
	auto Test       = StrToHex(R2.Data(), R2.Size());
	if (Test != HR) {
		Reader->SetError();
	}
}
