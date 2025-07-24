#pragma once
#include <pp_common/base.hpp>
#include <pp_protocol/cc_dv/device.hpp>
#include <pp_protocol/command.hpp>

struct xCC_Ipv6TestChennelReactor : xUdpChannel::iListener {
    void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
    void OnTerminalChallenge(xUdpChannel * ChannelPtr, const ubyte * Payload, size_t PayloadSize, const xNetAddress & RemoteAddress);
};

/////////////////////

struct xCC_ChallengeChennelReactor : xUdpChannel::iListener {
    void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
    void OnTerminalChallenge(xUdpChannel * ChannelPtr, const ubyte * Payload, size_t PayloadSize, const xNetAddress & RemoteAddress);
};