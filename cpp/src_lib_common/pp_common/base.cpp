#include "./base.hpp"

#include <crypto/sha.hpp>

std::string DebugSign(const void * DataPtr, size_t Size) {
    auto D = Sha256(DataPtr, Size);
    return StrToHex(D.Digest, 32);
}

uint32_t HashString(const char * S) {
    uint32_t H = 0;
    while (auto C = *(S++)) {
        H = H * 33 + C;
    }
    return H;
}

uint32_t HashString(const char * S, size_t Len) {
    uint32_t H = 0;
    for (size_t I = 0; I < Len; ++I) {
        H = H * 33 + S[I];
    }
    return H;
}

uint32_t HashString(const std::string & S) {
    return HashString(S.data(), S.size());
}
