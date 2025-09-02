#include "./device.hpp"

static const char * CheckAddressKey = "$%NDSFAS!";

xDeviceAddressPack xCC_DeviceChallenge::Extract() const {
    auto Result        = xDeviceAddressPack();
    auto Key           = std::string(CheckAddressKey);
    Result.Tcp4Address = ExtractAddressFromPack(Tcp4AddressKey, Key);
    Result.Tcp6Address = ExtractAddressFromPack(Tcp6AddressKey, Key);
    Result.Udp4Address = ExtractAddressFromPack(Udp4AddressKey, Key);
    Result.Udp6Address = ExtractAddressFromPack(Udp6AddressKey, Key);
    return Result;
}
