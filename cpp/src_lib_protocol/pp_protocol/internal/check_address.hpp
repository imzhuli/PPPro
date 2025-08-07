#pragma once
#include "../base.hpp"

struct xPP_AddressAutoPush : public xBinaryMessage {  // from proxy_access to relay server
public:
    void SerializeMembers() override {
        W(XR(ConnectionAddress.ToString()));  //
    }

    void DeserializeMembers() override {
        auto AddressString = std::string();
        R(AddressString);
        ConnectionAddress = xNetAddress::Parse(AddressString);
    }

    xNetAddress ConnectionAddress;
};
