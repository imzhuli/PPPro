#include "./network_security.hpp"

bool IsLocalNetAddress(const xNetAddress & Addr) {
    if (Addr.IsV4()) {
        ubyte B0 = Addr.SA4[0];
        ubyte B1 = Addr.SA4[1];
        if (B0 == 10) {
            return true;
        }
        if (B0 == 192 && B1 == 168) {
            return true;
        }
        if (B0 == 172 && (B1 >= 16 && B1 < 31)) {
            return true;
        }
    } else if (Addr.IsV6()) {
        ubyte B0 = Addr.SA6[0];
        ubyte B1 = Addr.SA6[1];
        if ((B0 & 0xFE) == 0xFC) {
            return true;
        }
        if (B0 == 0xFE && (B1 & 0xC0) == 0x80) {
            return true;
        }
    }
    return false;
}
