#pragma once
#include "../base.hpp"

/**
 * 接入服信息:
 *
 * FixedServerId: 由配置文件决定的SERVER_ID, 如果为0, 表示未设置
 * RuntimerServerId: 由服务器启动时, 通过向配置中心注册得到的运行时ID, 同一时间, 不同PA的ID必不同. 同一PA, 重启后的ID也会不同. 但如果配置中心重启,
 可能会出现重新拿到以前的ID的情况
 *

 */

struct xPA_BK_ServerInfo : xBinaryMessage {

    void SerializeMembers() override {
    }

    void DeserializeMembers() override {
    }

    uint64_t FixedServerId;
    uint64_t RuntimeServerId;
    uint64_t BindAddress;
    uint32_t CurrentClientS5ConnectionCount;
    uint32_t CurrentClientHttpConnectionCount;
    uint32_t CurrentClientUdpChannelCount;

    //
};
