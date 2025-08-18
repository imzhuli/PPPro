#include "./global.hpp"

void BroadCastRelayInfo(const xRelayServerInfoBase & Info) {
    DEBUG_LOG();

    // {
    //         auto Msg       = xPP_BroadcastRelayInfo();
    //         Msg.ServerInfo = Info;

    //         ubyte B[MaxPacketSize];
    //         auto  RS = WriteMessage(B, Cmd_BroadcastRelayInfo, 0, Msg);

    //         for (auto & P : SortedAvailableObserverConnections) {
    //             PostData(*P, B, RS);
    //         }
    //     }
}
