#pragma once
#include <pp_common/_.hpp>

class xRD_ConnectionBase : public xTcpConnection {
public:
    static constexpr const uint64_t FLAG_NONE = 0;

    static constexpr const uint64_t FLAG_TYPE_MASK    = 0x00FF;
    static constexpr const uint64_t FLAG_CTRL         = 0x01;
    static constexpr const uint64_t FLAG_DATA         = 0x02;
    static constexpr const uint64_t FLAG_PROXY_CLIENT = 0x04;

    static constexpr const uint64_t FLAG_MARK_MASK   = 0xFF00;
    static constexpr const uint64_t FLAG_MARK_DELETE = 0x0100;

public:
    uint64_t  ConnectionId   = 0;
    uint64_t  Flags          = FLAG_NONE;
    uint64_t  IdleTimestamMS = 0;
    uint64_t  TickCount      = 0;
    xVariable UserContext    = {};
    xVariable UserContextEx  = {};

    void ClearType() {
        Flags &= ~FLAG_TYPE_MASK;
    }
    bool IsType_Ctrl() const {
        return Flags & FLAG_CTRL;
    }
    void SetType_Ctrl() {
        ClearType();
        Flags |= FLAG_CTRL;
    }
    bool IsType_Data() const {
        return Flags & FLAG_DATA;
    }
    void SetType_Data() {
        ClearType();
        Flags |= FLAG_DATA;
    }
    bool IsType_ProxyClient() const {
        return Flags & FLAG_PROXY_CLIENT;
    }
    void SetType_ProxyClient() {
        ClearType();
        Flags |= FLAG_PROXY_CLIENT;
    }

    // marks
    void ClearMark() {
        Flags &= ~FLAG_MARK_MASK;
    }
    bool HasMark_Delete() const {
        return Flags & FLAG_MARK_DELETE;
    }
    void Mark_Delete() {
        Flags |= FLAG_MARK_DELETE;
    }

    bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
};