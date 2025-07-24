#include "./internal_key.hpp"

std::string MakeInternalKey(const xNetAddress & InternalIpv4Address, const xNetAddress & InternalIpv6Address, const std::string & CheckKey) {
    auto PK = xInitDataKeyPacker();

    PK.InternalIpv4Address = InternalIpv4Address;
    PK.InternalIpv6Address = InternalIpv6Address;
    PK.CheckKey            = CheckKey;

    ubyte  Buffer[2000];
    size_t RSize = PK.Serialize(Buffer, sizeof(Buffer));
    RuntimeAssert(RSize);
    return StrToHex((const char *)Buffer, RSize);
}
