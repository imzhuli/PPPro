#include "./challenge_service.hpp"

#include "./_global.hpp"

void xCC_Ipv6TestChennelReactor::OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) {
    auto AddrStr = RemoteAddress.ToString();

    if (DataSize < PacketHeaderSize) {
        DEBUG_LOG("Invalid packet size");
        return;
    }
    auto Header = xPacketHeader();
    Header.Deserialize(DataPtr);
    if (Header.PacketSize != DataSize) {
        DEBUG_LOG("Invalid packet size from header");
        return;
    }

    DEBUG_LOG("Request: CmdId=%" PRIx32 ", RequestId=%" PRIx64 ", RemoteAddress=%s", Header.CommandId, Header.RequestId, RemoteAddress.ToString().c_str());
    auto Payload     = xPacket::GetPayloadPtr(DataPtr);
    auto PayloadSize = Header.GetPayloadSize();
    Touch(PayloadSize);

    DEBUG_LOG("Request body: \n%s", HexShow(Payload, PayloadSize).c_str());
    switch (Header.CommandId) {
        case Cmd_DV_CC_Challenge: {
            OnTerminalChallenge(ChannelPtr, Payload, PayloadSize, RemoteAddress);
            break;
        }

        default:
            DEBUG_LOG("Unrecognized command");
            break;
    }
    return;
};

void xCC_Ipv6TestChennelReactor::OnTerminalChallenge(xUdpChannel * ChannelPtr, const ubyte * Payload, size_t PayloadSize, const xNetAddress & RemoteAddress) {
    auto Request = xCC_DeviceChallenge();
    if (!Request.Deserialize(Payload, PayloadSize)) {
        DEBUG_LOG("Invalid requst format");
    }

    DEBUG_LOG("Version:%" PRIu32 ", Timestamp:%" PRIu64 ", Sign=%s", Request.AppVersion, Request.Timestamp, Request.Sign.c_str());
    // check sign:
    auto Source = "TLMPP1" + std::to_string(Request.Timestamp);
    auto Digest = Md5(Source.data(), Source.size());
    if (Request.Sign != StrToHex(Digest.Data(), Digest.Size())) {
        DEBUG_LOG("Invalid sign");
        return;
    }

    DEBUG_LOG("Challenge accepted");
    auto Resp            = xCC_DeviceChallengeResp();
    Resp.TerminalAddress = RemoteAddress.Ip();

    ubyte Buffer[MaxPacketSize];
    auto  RSize = WriteMessage(Buffer, Cmd_DV_CC_ChallengeResp, 0, Resp);

    DEBUG_LOG("Post Response to %s\n%s", RemoteAddress.ToString().c_str(), HexShow(Buffer, RSize).c_str());
    ChannelPtr->PostData(RemoteAddress, Buffer, RSize);
}

///////////

void xCC_ChallengeChennelReactor::OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) {
    auto AddrStr = RemoteAddress.ToString();
    cout << "ipv6 chnallenge :" << endl << HexShow(AddrStr) << endl;

    if (DataSize < PacketHeaderSize) {
        DEBUG_LOG("Invalid packet size");
        return;
    }
    auto Header = xPacketHeader();
    Header.Deserialize(DataPtr);
    if (Header.PacketSize != DataSize) {
        DEBUG_LOG("Invalid packet size from header");
        return;
    }

    DEBUG_LOG("Request: CmdId=%" PRIx32 ", RequestId=%" PRIx64 ", RemoteAddress=%s", Header.CommandId, Header.RequestId, RemoteAddress.ToString().c_str());
    auto Payload     = xPacket::GetPayloadPtr(DataPtr);
    auto PayloadSize = Header.GetPayloadSize();
    Touch(PayloadSize);

    DEBUG_LOG("Request body: \n%s", HexShow(Payload, PayloadSize).c_str());
    switch (Header.CommandId) {
        case Cmd_DV_CC_Challenge: {
            OnTerminalChallenge(ChannelPtr, Payload, PayloadSize, RemoteAddress);
            break;
        }

        default:
            DEBUG_LOG("Unrecognized command");
            break;
    }
    return;
};

void xCC_ChallengeChennelReactor::OnTerminalChallenge(xUdpChannel * ChannelPtr, const ubyte * Payload, size_t PayloadSize, const xNetAddress & RemoteAddress) {
    auto AddrStr = RemoteAddress.ToString();
    cout << "chnallenge data:" << endl << HexShow(AddrStr) << endl;

    auto Request = xCC_DeviceChallenge();
    if (!Request.Deserialize(Payload, PayloadSize)) {
        DEBUG_LOG("Invalid requst format");
    }

    DEBUG_LOG("Version:%" PRIu32 ", Timestamp:%" PRIu64 ", Sign=%s", Request.AppVersion, Request.Timestamp, Request.Sign.c_str());
    // check sign:
    auto Source = "TLMPP1" + std::to_string(Request.Timestamp);
    auto Digest = Md5(Source.data(), Source.size());
    if (Request.Sign != StrToHex(Digest.Data(), Digest.Size())) {
        DEBUG_LOG("Invalid sign");
        return;
    }

    DEBUG_LOG("Challenge accepted, checking relay server nodes");
    auto RID = IpLocationManager.GetRegionByIp(RemoteAddress.IpToString().c_str());
    auto RSI = GetRandomRelayServerInfoByRegion(RID);

    auto Resp            = xCC_DeviceChallengeResp();
    Resp.TerminalAddress = RemoteAddress.Ip();

    ubyte RegionBuffer[64];
    auto  RegionWriter = xStreamWriter(RegionBuffer);
    RegionWriter.W4(RID.CountryId);
    RegionWriter.W4(RID.StateId);
    RegionWriter.W4(RID.CityId);
    if (RSI) {
        Resp.Address  = RSI->ExportDeviceAddress4;
        Resp.CheckKey = "TLMPP-FOR-TEST:" + StrToHex(RegionWriter.Origin(), RegionWriter.Offset());
    }

    ubyte Buffer[MaxPacketSize];
    auto  RSize = WriteMessage(Buffer, Cmd_DV_CC_ChallengeResp, 0, Resp);

    DEBUG_LOG("Post Response to %s\n%s", RemoteAddress.ToString().c_str(), HexShow(Buffer, RSize).c_str());
    ChannelPtr->PostData(RemoteAddress, Buffer, RSize);
};
