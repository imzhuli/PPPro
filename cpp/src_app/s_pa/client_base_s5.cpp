#include "./_global.hpp"

size_t xPA_ClientStateHandler_S5_Challenge::OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
    assert(DataSize >= 3);
    auto R = xStreamReader(DataPtr);
    R.Skip(1);  // skip type check bytes

    size_t NM = R.R1();  // number of methods
    if (!NM) {
        return 0;
    }
    size_t HeaderSize = 2 + NM;
    if (DataSize < HeaderSize) {
        return 0;
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
        // if (Method == 0x00) {
        //     NoAuthSupport = true;
        //     continue;
        // }
    }
    if (UserPassSupport) {
        ubyte Socks5Auth[2] = { 0x05, 0x02 };
        Client->PostData(Socks5Auth, sizeof(Socks5Auth));
    } else if (NoAuthSupport) {
        ubyte Socks5Auth[2] = { 0x05, 0x00 };
        Client->PostData(Socks5Auth, sizeof(Socks5Auth));
    } else {
        DEBUG_LOG("Unsupported auth method");
        ubyte Socks5Auth[2] = { 0x05, 0xFF };
        Client->PostData(Socks5Auth, sizeof(Socks5Auth));
        ScheduleClientStateChange(Client, CloseClientStateHandler);
        return HeaderSize;
    }

    ScheduleClientStateChange(Client, S5WaitForAuthInfoStateHandler);
    return HeaderSize;
}

size_t xPA_ClientStateHandler_S5_WaitForAuthInfo::OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
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
    } else {
        DEBUG_LOG("AuthMethod: %u", (unsigned)Ver);
        ScheduleClientStateChange(Client, CloseClientStateHandler);
        return InvalidDataSize;
    }
    DEBUG_LOG("AuthInfo AuthType=%u: NP=%s", (unsigned)Ver, NP.c_str());

    AuthCacheLocalServer.PostAuthRequest(Client->ConnectionId, NP);
    ScheduleClientStateChange(Client, S5WaitForAuthResultStateHandler);
    return R.Offset();
}

size_t xPA_ClientStateHandler_S5_WaitForAuthResult::OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
    if (!DataSize) {
        return 0;
    }

    DEBUG_LOG("Forward data received, closing connection, data=\n%s", HexShow(DataPtr, DataSize).c_str());
    ScheduleClientStateChange(Client, CloseClientStateHandler);
    return 0;
};
