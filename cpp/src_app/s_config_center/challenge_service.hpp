#pragma once
#include <pp_common/_.hpp>
#include <pp_protocol/cc_dv/device.hpp>
#include <pp_protocol/command.hpp>

extern void OnTerminalChallenge(const xUdpServiceChannelHandle & Handle, xPacketCommandId CmdId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);
