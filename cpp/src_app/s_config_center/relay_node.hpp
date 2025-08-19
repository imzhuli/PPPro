#pragma once
#include <pp_common/_.hpp>
#include <pp_protocol/cc_dv/device.hpp>
#include <pp_protocol/command.hpp>

struct xCC_RelayV4Node : xListNode {};
using xCC_RelayV4List = xList<xCC_RelayV4Node>;

struct xCC_RelayV6Node : xListNode {};
using xCC_RelayV6List = xList<xCC_RelayV6Node>;

struct xCC_RelayScheduleNode
    : xCC_RelayV4Node
    , xCC_RelayV6Node {
    const xRelayServerInfoBase * ServerInfo;
};
