#include "./_global.hpp"
#include "./client_manager.hpp"

size_t OnPAC_S5_Challenge(xPA_ClientConnection * Client, ubyte * DP, size_t DataSize) {
    assert(Client->State == CS_S5_CHALLENGE);
    if (DataSize < 3) {
        return 0;
    }
    auto R = xStreamReader(DP);
    R.Skip(1);  // skip type check bytes

    size_t NM = R.R1();  // number of methods
    if (!NM) {           // no auth methods
        return InvalidDataSize;
    }
    size_t HeaderSize = 2 + NM;
    if (DataSize < HeaderSize) {
        return 0;  // require more data
    }

    bool UserPassSupport = false;
    bool NoAuthSupport   = false;
    Touch(NoAuthSupport);
    for (size_t i = 0; i < NM; ++i) {
        uint8_t Method = R.R1();
        if (Method == 0x02) {
            UserPassSupport = true;
            continue;
        }
        if (Method == 0x00) {
            NoAuthSupport = true;
            continue;
        }
    }
    if (UserPassSupport) {
        DEBUG_LOG("user pass requried");
        ubyte Socks5Auth[2] = { 0x05, 0x02 };
        Client->PostData(Socks5Auth, sizeof(Socks5Auth));
        Client->State = CS_S5_WAIT_FOR_AUTH_INFO;
    } else if (NoAuthSupport) {
        Client->State = CS_S5_WAIT_FOR_AUTH_INFO;
        // ubyte Socks5NoAuthAccepted[2] = { 0x05, 0x00 };
        // Client->PostData(Socks5Auth, sizeof(Socks5NoAuthAccepted));
    } else {
        DEBUG_LOG("Unsupported auth method");
        ubyte Socks5Auth[2] = { 0x05, 0xFF };
        Client->PostData(Socks5Auth, sizeof(Socks5Auth));
        SchedulePassiveKillClientConnection(Client);
    }
    return HeaderSize;
}

size_t OnPAC_S5_AuthInfo(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
    DEBUG_LOG("");
    if (DataSize < 3) {
        return 0;
    }

    auto R   = xStreamReader(DataPtr);
    auto Ver = R.R1();
    auto NP  = Client->GetRemoteAddress().IpToString();
    if (Ver == 0x01) {  // only version for user pass
        size_t NameLen = R.R1();
        if (DataSize < 3 + NameLen) {
            return 0;
        }
        // auto   NameView = std::string_view((const char *)R(), NameLen);
        char * KeyStart = (char *)DataPtr + R.Offset();
        R.Skip(NameLen);

        size_t PassLen = R.R1();
        if (DataSize < 3 + NameLen + PassLen) {
            DEBUG_LOG("Wait for auth data");
            return 0;
        }
        // auto PassView                     = std::string_view((const char *)R(), PassLen);
        ((char *)DataPtr)[R.Offset() - 1] = ':';  // make quick user/pass key
        R.Skip(PassLen);
        if (NameLen || PassLen) {
            NP = std::string{ KeyStart, NameLen + PassLen + 1 };
        } else {
            DEBUG_LOG("Empty user:pass found, using IP");
        }
    } else if (Ver == 0x00) {  // No Auth
        DEBUG_LOG("Empty S5 AuthInfo, using IP");
        return InvalidDataSize;
    } else {
        DEBUG_LOG("AuthMethod: %u", (unsigned)Ver);
        return InvalidDataSize;
    }
    DEBUG_LOG("AuthInfo AuthType=%u: NP=%s", (unsigned)Ver, NP.c_str());
    if (!PostAuthRequest(Client->Connectionid, NP)) {
        Client->PostData("\x01\x01", 2);
        SchedulePassiveKillClientConnection(Client);
    } else {
        Client->State = CS_S5_WAIT_FOR_AUTH_RESULT;
    }
    return R.Offset();
}

size_t OnPAC_S5_TargetAddress(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
    if (DataSize < 10) {
        return 0;
    }
    if (DataSize >= 6 + 256) {
        DEBUG_LOG("Very big connection request, which is obviously wrong");
        DeferKillClientConnection(Client);
        return 0;
    }
    xStreamReader R(DataPtr);
    uint8_t       Version   = R.R();
    uint8_t       Operation = R.R();
    uint8_t       Reserved  = R.R();
    uint8_t       AddrType  = R.R();
    if (Version != 0x05 || Reserved != 0x00) {
        DEBUG_LOG("Non Socks5 connection request");
        DeferKillClientConnection(Client);
        return 0;
    }
    xNetAddress Address;
    char        DomainName[256];
    size_t      DomainNameLength = 0;
    if (AddrType == 0x01) {  // ipv4
        Address.Type = xNetAddress::IPV4;
        R.R(Address.SA4, 4);
        Address.Port = R.R2();
    } else if (AddrType == 0x03) {
        DomainNameLength = R.R();
        if (DataSize < 4 + 1 + DomainNameLength + 2) {
            return 0;
        }
        R.R(DomainName, DomainNameLength);
        DomainName[DomainNameLength] = '\0';
        Address.Port                 = R.R2();
    } else if (AddrType == 0x04) {  // ipv6
        if (DataSize < 4 + 16 + 2) {
            return 0;
        }
        Address.Type = xNetAddress::IPV6;
        R.R(Address.SA6, 16);
        Address.Port = R.R2();
    } else {
        DEBUG_LOG("Invalid connection request");
        DeferKillClientConnection(Client);
        return 0;
    }
    size_t AddressLength = R.Offset() - 3;
    if (Operation != 0x01 || AddrType == 0x06) {
        DEBUG_LOG("Operation other than tcp ipv4/domain connection");
        ubyte         Buffer[512];
        xStreamWriter W(Buffer);
        W.W(0x05);
        W.W(0x01);
        W.W(0x00);
        W.W((ubyte *)DataPtr + 3, AddressLength);
        Client->PostData(Buffer, W.Offset());
        SchedulePassiveKillClientConnection(Client);
        return 0;
    }

    DEBUG_LOG("TargetAddress: NA:%s, Host:%s", Address.ToString().c_str(), DomainName);
    return 0;
}

void OnPAC_S5_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR) {
    if (!AR) {
        CC->PostData("\x01\x01", 2);
        SchedulePassiveKillClientConnection(CC);
    }
    CC->PostData("\x01\x00", 2);
    KeepAlive(CC);

    CC->State = CS_S5_WAIT_FOR_TARGET_ADDRESS;
}

// size_t xPA_ClientStateHandler_S5_WaitForAuthResult::OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
//     if (!DataSize) {
//         return 0;
//     }

//     DEBUG_LOG("Forward data received, closing connection, data=\n%s", HexShow(DataPtr, DataSize).c_str());
//     ScheduleClientStateChange(Client, CloseClientStateHandler);
//     return 0;
// };
