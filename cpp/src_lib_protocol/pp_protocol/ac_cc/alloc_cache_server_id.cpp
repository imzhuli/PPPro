#include "./alloc_cache_server_id.hpp"

#include <crypto/md5.hpp>

static constexpr const char SALT_BUFFER[] = "xdfasd56YKJ";

static xMd5Result Hash(uint64_t TimestampMS) {
    char HBuffer[100];
    auto PR = xStreamWriter(HBuffer);
    PR.W(SALT_BUFFER, sizeof(SALT_BUFFER));
    PR.W(TimestampMS);
    return Md5(HBuffer, PR.Offset());
}

void x_AC_CC_AllocServerId::SerializeMembers() {
    auto HResult = Hash(TimestampMS);
    W(TimestampMS);
    W(std::string_view{ (const char *)HResult.Digest, 16 });
};

void x_AC_CC_AllocServerId::DeserializeMembers() {
    std::string Check;
    R(TimestampMS);
    R(Check);

    auto HResult = Hash(TimestampMS);
    if (Check != std::string_view((const char *)HResult.Digest, 16)) {
        X_DEBUG_PRINTF("Invalid checksum");
        xBinaryMessageReader::SetError();
        return;
    }
};
