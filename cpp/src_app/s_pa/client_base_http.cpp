#include "./_global.hpp"

// size_t xPA_ClientStateHandler_Http_Challenge::OnDataEvent(xPA_ClientConnection * Client, ubyte * DataPtr, size_t DataSize) {
//     assert(DataSize);

//     auto FirstByte = (char)(DataPtr)[0];
//     if (FirstByte == '\x05') {
//         ScheduleClientStateChange(Client, S5ChallengeStateHandler);
//         return 0;
//     }

//     return 0;
// }
