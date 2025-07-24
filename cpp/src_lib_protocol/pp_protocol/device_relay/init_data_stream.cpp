#include "./init_data_stream.hpp"

#include "./internal_key.hpp"

bool xInitDataStream::UnpackDataKey(const std::string & Key) {
    auto BK = HexToStr(Key);

    auto PK = xInitDataKeyPacker();
    if (!PK.Deserialize(BK.data(), BK.size())) {
        return false;
    }

    InternalIpv4Address = PK.InternalIpv4Address;
    InternalIpv6Address = PK.InternalIpv6Address;
    DataKey             = PK.CheckKey;
    return true;
}

xOptional<std::string> xInitDataStream::PackDataKey() {
    auto PK = xInitDataKeyPacker();

    PK.InternalIpv4Address = InternalIpv4Address;
    PK.InternalIpv6Address = InternalIpv6Address;
    PK.CheckKey            = DataKey;

    ubyte  Buffer[200];
    size_t RSize = PK.Serialize(Buffer, sizeof(Buffer));
    if (RSize) {
        return std::string((const char *)Buffer, RSize);
    }
    return {};
}
