#include "./connection.hpp"

/***************
 * Connection
 */
bool xRD_ConnectionBase::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
    ubyte Buffer[MaxPacketSize];
    auto  PSize = WriteMessage(Buffer, CmdId, RequestId, Message);
    if (!PSize) {
        return false;
    }
    PostData(Buffer, PSize);
    return true;
}
