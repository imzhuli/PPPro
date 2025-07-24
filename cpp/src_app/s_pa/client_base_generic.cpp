#include "./_global.hpp"

size_t xPA_ClientStateHandler_TestProxyType::OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
    assert(DataSize);
    Logger->I("DataEvent, xPA_ClientStateHandler_TestProxyType");

    if (DataSize < 3) {  // not enough header size:
        return 0;
    }

    auto FirstByte = (char)(DataPtr)[0];
    if (FirstByte == '\x05') {
        ScheduleClientStateChange(Client, S5ChallengeStateHandler);
        return 0;
    }
    ScheduleClientStateChange(Client, HttpChallengeStateHandler);
    return 0;
}

void xPA_ClientStateHandler_CloseStateHandler::OnEntry(xPA_ClientConnection * Client) {
    KillConnection(Client);
}