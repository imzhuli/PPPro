#include "./device_manager.hpp"

#include "../lib_utils/all.hpp"
#include "./_global.hpp"
#include "pp_protocol/device_relay/connection.hpp"
#include "pp_protocol/device_relay/post_data.hpp"
#include "pp_protocol/device_relay/udp_channel.hpp"

#include <pp_protocol/device_relay/handshake.hpp>

xIndexedStorage<xDR_DeviceContext> DeviceManager;

//////

static xDR_DeviceContext * CreateDeviceContext(const xTcpServiceClientConnectionHandle & Handle) {
    auto Id = DeviceManager.Acquire({ 0, Handle });
    if (!Id) {
        return nullptr;
    }
    auto & Ref = DeviceManager[Id];
    Ref.Id     = Id;
    return &Ref;
}

static void ReleaseDeviceContext(uint64_t Id) {
    DeviceManager.Release(Id);
}

static bool OnDeviceCreateConnectionResp(xDR_DeviceContext * PDC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Resp = xPP_CreateConnectionResp();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invlid protocol");
        return false;
    }
    auto PRC = GetRelayContextById(Resp.RelaySideContextId);
    if (!PRC) {
        DEBUG_LOG("Missing relay context");
        return true;
    }
    DEBUG_LOG("CreateConnectionResult: %s, DeviceSideContextId=%" PRIx32 "", YN(Resp.Connected), Resp.DeviceSideContextId);

    // TODO:
    if (!Resp.Connected) {
        ReleaseRelayContext(PRC);
        // TODO: notify proxy

    } else {
        PRC->DeviceSideContextId = Resp.DeviceSideContextId;
        // TEST:
        auto Req = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nUser-Agent: curl/8.5.0\r\n\r\n";
        PostConnectionData(PRC->RelaySideContextId, Req, strlen(Req));
    }

    return true;
}

static bool OnDeviceDestroyConnection(xDR_DeviceContext * PDC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Resp = xPP_DestroyConnection();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invlid protocol");
        return false;
    }
    auto PRC = GetRelayContextById(Resp.RelaySideContextId);
    if (!PRC) {
        DEBUG_LOG("Missing relay context");
        return true;
    }
    DEBUG_LOG("OnDeviceDestroyConnection DeviceSideContextId=%" PRIx32 "", Resp.DeviceSideContextId);

    // TODO: notify proxy

    ReleaseRelayContext(PRC);
    return true;
}

static bool OnDeviceCreateUdpChannelResp(xDR_DeviceContext * PDC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Resp = xPP_CreateUdpChannelResp();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invlid protocol");
        return false;
    }
    auto PRC = GetRelayContextById(Resp.RelaySideContextId);
    if (!PRC) {
        DEBUG_LOG("Missing relay context");
        return true;
    }
    DEBUG_LOG("CreateUdpChannelResult: DeviceSideContextId=%" PRIx32 "", Resp.DeviceSideContextId);

    // TODO:
    if (!Resp.DeviceSideContextId) {
        ReleaseRelayContext(PRC);
        // TODO: notify proxy

    } else {
        PRC->DeviceSideContextId = Resp.DeviceSideContextId;

        // test
        auto   TP = xPP_UdpChannelKeepAlive();
        ubyte  BF[xel::MaxPacketSize];
        size_t RSize = xel::WriteMessage(BF, 1, 2, TP);

        PostUdpChannelData(PRC->RelaySideContextId, xNetAddress::Parse("[2402:4e00:101a:f300:0:9f95:4b15:c0db]:9000"), BF, RSize);
        PostUdpChannelData(PRC->RelaySideContextId, xNetAddress::Parse("127.0.0.1:9000"), BF, RSize);
    }

    return true;
}

static bool OnDeviceDestroyUdpChannel(xDR_DeviceContext * PDC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Resp = xPP_DestroyUdpChannel();
    if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invlid protocol");
        return false;
    }
    auto PRC = GetRelayContextById(Resp.RelaySideContextId);
    if (!PRC) {
        DEBUG_LOG("Missing relay context");
        return true;
    }
    DEBUG_LOG("OnDeviceDestroyUdpBinding DeviceSideContextId=%" PRIx32 "", Resp.DeviceSideContextId);

    // TODO: notify proxy

    ReleaseRelayContext(PRC);
    return true;
}

static bool OnDevicePostConnectionData(xDR_DeviceContext * PDC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto PP = xPP_PostConnectionData();
    if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invlid protocol");
        return false;
    }
    auto PRC = GetRelayContextById(PP.RelaySideContextId);
    if (!PRC) {
        DEBUG_LOG("Missing relay context");
        return true;
    }

    DEBUG_LOG("%s", HexShow(PP.PayloadView).c_str());

    // TODO:

    return true;
}

static bool OnDevicePostUdpChannelData(xDR_DeviceContext * PDC, ubyte * PayloadPtr, size_t PayloadSize) {
    auto PP = xPP_PostUdpChannelData();
    if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invlid protocol");
        return false;
    }
    auto PRC = GetRelayContextById(PP.RelaySideContextId);
    if (!PRC) {
        DEBUG_LOG("Missing relay context");
        return true;
    }

    DEBUG_LOG("RemoteAddress:%s\n%s", PP.TargetAddress.ToString().c_str(), HexShow(PP.PayloadView).c_str());

    // TODO:

    return true;
}

static bool OnDevicePacket(xDR_DeviceContext * PDC, xPacketCommandId CmdId, xPacketRequestId ReqId, ubyte * PayloadPtr, size_t PayloadSize) {

    DEBUG_LOG("%" PRIx32 "/%" PRIx64 " message_length=%z", PayloadSize);

    switch (CmdId) {
        case Cmd_DV_RL_CreateConnectionResp: {
            return OnDeviceCreateConnectionResp(PDC, PayloadPtr, PayloadSize);
        }
        case Cmd_DV_RL_DestroyConnection: {
            return OnDeviceDestroyConnection(PDC, PayloadPtr, PayloadSize);
        }
        case Cmd_DV_RL_PostConnectionData: {
            return OnDevicePostConnectionData(PDC, PayloadPtr, PayloadSize);
        }
        case Cmd_DV_RL_CreateUdpChannelResp: {
            return OnDeviceCreateUdpChannelResp(PDC, PayloadPtr, PayloadSize);
        }
        case Cmd_DV_RL_DestroyUdpChannel: {
            return OnDeviceDestroyUdpChannel(PDC, PayloadPtr, PayloadSize);
        }
        case Cmd_DV_RL_PostUdpChannelData: {
            return OnDevicePostUdpChannelData(PDC, PayloadPtr, PayloadSize);
        }
    }
    return true;
}

static bool OnDeviceHandshake(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DeviceHandshake();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invlid protocol");
        return false;
    }
    auto PDC = CreateDeviceContext(Handle);
    if (!PDC) {
        return false;
    }
    Handle->UserContext.U64 = PDC->Id;

    auto RS     = xPP_DeviceHandshakeResp();
    RS.Accepted = true;
    Handle.PostMessage(Cmd_DV_RL_HandshakeResp, 0, RS);
    DEBUG_LOG("accept new device connection: %" PRIx64 ", remote address=%s", PDC->Id, Handle.GetRemoteAddress().ToString().c_str());

    CreateTcpConnection(PDC->Id, "www.baidu.com", 80);
    CreateUdpChannel4(PDC->Id);
    return true;
}

void InitDeviceContextManager() {
    xel::RuntimeAssert(DeviceManager.Init(2 * MaxDeviceCount));
}

void CleanDeviceContextManager() {
    DeviceManager.Clean();
}

xDR_DeviceContext * GetDeviceContextById(uint64_t Id) {
    return DeviceManager.CheckAndGet(Id);
}

void OnDeviceConnectionClean(const xTcpServiceClientConnectionHandle & Handle) {
    auto Id = Handle->UserContext.U64;
    if (!Id) {
        DEBUG_LOG("remove uninited device connection");
        return;
    }
    DEBUG_LOG("remove handshake ready device connection: %" PRIx64 "", Id);
    ReleaseDeviceContext(Id);
}

bool OnDeviceConnectionPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CmdId, xPacketRequestId ReqId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Id = Handle->UserContext.U64;
    if (!Id) {
        if (CmdId != Cmd_DV_RL_Handshake) {
            return false;
        }
        return OnDeviceHandshake(Handle, PayloadPtr, PayloadSize);
    }
    auto PDC = GetDeviceContextById(Id);
    assert(PDC);
    return OnDevicePacket(PDC, CmdId, ReqId, PayloadPtr, PayloadSize);
}
