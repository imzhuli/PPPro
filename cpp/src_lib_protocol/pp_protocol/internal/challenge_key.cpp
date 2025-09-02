#include "./challenge_key.hpp"

struct xDeviceAddressPackMessage_W : xBinaryMessage {

    void SerializeMembers() override {
        W(Pack->Tcp4Address);
        W(Pack->Tcp6Address);
        W(Pack->Udp4Address);
        W(Pack->Udp6Address);
    }
    void DeserializeMembers() override {}

    const xDeviceAddressPack * Pack = nullptr;
};

struct xDeviceAddressPackMessage_R : xBinaryMessage {

    void DeserializeMembers() override {
        R(Pack->Tcp4Address);
        R(Pack->Tcp6Address);
        R(Pack->Udp4Address);
        R(Pack->Udp6Address);
    }

    xDeviceAddressPack * Pack = nullptr;
};

std::string MakeChallengeKey(const xDeviceAddressPack & AddressPack) {
    auto M = xDeviceAddressPackMessage_W();
    M.Pack = &AddressPack;

    ubyte  B[MaxPacketSize];
    size_t S = M.Serialize(B, sizeof(B));

    auto H = StrToHex(B, S);

    return H;
}

xDeviceAddressPack ExtractChallengeKey(const std::string & Key) {
    auto R  = xDeviceAddressPack();
    auto RM = xDeviceAddressPackMessage_R();
    RM.Pack = &R;

    auto S = xel::HexToStr(Key);
    if (!RM.Deserialize(S.data(), S.size())) {
        return {};
    }
    return R;
}
