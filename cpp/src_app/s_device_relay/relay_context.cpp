#include "./relay_context.hpp"

#include "./_global.hpp"
#include "pp_protocol/device_relay/connection.hpp"
#include "pp_protocol/device_relay/post_data.hpp"
#include "pp_protocol/device_relay/udp_channel.hpp"

#include <atomic>

static xel::xIndexedStorage<xRL_RelayContext> RelayContextPool;
static xRL_RelayContextTimeoutList            RelayContextIdleList;

void InitRelayContextPool() {
    RuntimeAssert(RelayContextPool.Init(MaxRelayContextCount));
}

void CleanRelayContextPool() {
    RelayContextPool.Clean();
}

static xRL_RelayContext * AllocRelayContext(uint64_t DeviceId) {
    auto Id = RelayContextPool.AcquireValue();
    if (!Id) {
        return nullptr;
    }
    auto & RC             = RelayContextPool[Id];
    RC.RelaySideContextId = Id;
    RC.DeviceId           = DeviceId;
    RC.TimestampMS        = ServiceTicker();
    RelayContextIdleList.AddTail(RC);
    return &RC;
}

void ReleaseRelayContext(xRL_RelayContext * PRC) {
    assert(RelayContextPool.CheckAndGet(PRC->RelaySideContextId) == PRC);
    DEBUG_LOG("ContextId=%" PRIx64 "", PRC->RelaySideContextId);
    RelayContextPool.Release(PRC->RelaySideContextId);
}

xRL_RelayContext * GetRelayContextById(uint64_t RelaySideContextId) {
    return RelayContextPool.CheckAndGet(RelaySideContextId);
}

void KeepAlive(xRL_RelayContext * PRC) {
    PRC->TimestampMS = ServiceTicker();
    RelayContextIdleList.GrabTail(*PRC);
}

void ReleaseTimeoutRelayContext() {
    auto KillTimepoint = ServiceTicker() - RelayContextIdleTimeoutMS;
    while (auto PRC = static_cast<xRL_RelayContext *>(RelayContextIdleList.PopHead([KillTimepoint](auto & C) { return C.TimestampMS <= KillTimepoint; }))) {
        ReleaseRelayContext(PRC);
    }
}

xRL_RelayContext * CreateTcpConnection(uint64_t DeviceId, const xNetAddress & TargetAddress) {
    auto PDC = GetDeviceContextById(DeviceId);
    if (!PDC) {
        DEBUG_LOG("invalid device id: %" PRIx64 "", DeviceId);
        return nullptr;
    }
    auto PRC = AllocRelayContext(DeviceId);
    if (!PRC) {
        DEBUG_LOG("failed to alloc relay context");
        return nullptr;
    }

    auto CC               = xPP_CreateConnection();
    CC.RelaySideContextId = PRC->RelaySideContextId;
    CC.TargetAddress      = TargetAddress;
    PDC->Handle.PostMessage(Cmd_DV_RL_CreateConnection, 0, CC);
    PRC->ContextState = xRelayContextState::TCP_CONNECTING;
    return PRC;
}

xRL_RelayContext * CreateTcpConnection(uint64_t DeviceId, const std::string_view & HostnameView, uint16_t Port) {
    auto PDC = GetDeviceContextById(DeviceId);
    if (!PDC) {
        DEBUG_LOG("invalid device id: %" PRIx64 "", DeviceId);
        return nullptr;
    }
    auto PRC = AllocRelayContext(DeviceId);
    if (!PRC) {
        DEBUG_LOG("failed to alloc relay context");
        return nullptr;
    }

    //
    auto CC               = xPP_CreateConnectionHost();
    CC.RelaySideContextId = PRC->RelaySideContextId;
    CC.HostnameView       = HostnameView;
    CC.Port               = Port;
    PDC->Handle.PostMessage(Cmd_DV_RL_CreateConnectionHost, 0, CC);

    PRC->ContextState = xRelayContextState::TCP_CONNECTING;
    return PRC;
}

xRL_RelayContext * CreateUdpChannel(uint64_t DeviceId) {
    auto PDC = GetDeviceContextById(DeviceId);
    if (!PDC) {
        return nullptr;
    }
    auto PRC = AllocRelayContext(DeviceId);
    if (!PRC) {
        return nullptr;
    }

    auto CUB               = xPP_CreateUdpChannel();
    CUB.RelaySideContextId = PRC->RelaySideContextId;
    CUB.Enable4            = true;
    CUB.Enable6            = true;
    PDC->Handle.PostMessage(Cmd_DV_RL_CreateUdpChannel, 0, CUB);
    PRC->ContextState = xRelayContextState::UDP_BINDING;
    return PRC;
}

xRL_RelayContext * CreateUdpChannel4(uint64_t DeviceId) {
    auto PDC = GetDeviceContextById(DeviceId);
    if (!PDC) {
        return nullptr;
    }
    auto PRC = AllocRelayContext(DeviceId);
    if (!PRC) {
        return nullptr;
    }
    auto CUB               = xPP_CreateUdpChannel();
    CUB.RelaySideContextId = PRC->RelaySideContextId;
    CUB.Enable4            = true;
    PDC->Handle.PostMessage(Cmd_DV_RL_CreateUdpChannel, 0, CUB);
    PRC->ContextState = xRelayContextState::UDP_BINDING;
    return PRC;
}

xRL_RelayContext * CreateUdpChannel6(uint64_t DeviceId) {
    auto PDC = GetDeviceContextById(DeviceId);
    if (!PDC) {
        return nullptr;
    }
    auto PRC = AllocRelayContext(DeviceId);
    if (!PRC) {
        return nullptr;
    }
    auto CUB               = xPP_CreateUdpChannel();
    CUB.RelaySideContextId = PRC->RelaySideContextId;
    CUB.Enable6            = true;
    PDC->Handle.PostMessage(Cmd_DV_RL_CreateUdpChannel, 0, CUB);
    PRC->ContextState = xRelayContextState::UDP_BINDING;
    return PRC;
}

void PostConnectionData(uint64_t RelaySideContextId, const void * DataPtr, size_t DataSize) {
    auto PRC = GetRelayContextById(RelaySideContextId);
    if (!PRC) {
        return;
    }
    auto PDC = GetDeviceContextById(PRC->DeviceId);
    if (!PDC || !PDC->Handle.IsValid()) {
        return;
    }
    const char * PB = static_cast<const char *>(DataPtr);
    while (DataSize) {
        auto PostSize           = std::min(DataSize, xPP_PostConnectionData::MAX_PAYLOAD_SIZE);
        auto Req                = xPP_PostConnectionData();
        Req.DeviceSideContextId = PRC->DeviceSideContextId;
        Req.RelaySideContextId  = PRC->RelaySideContextId;
        Req.PayloadView         = { PB, PostSize };
        PDC->Handle.PostMessage(Cmd_DV_RL_PostConnectionData, 0, Req);

        PB       += PostSize;
        DataSize -= PostSize;
    }
    return;
}

void PostUdpChannelData(uint64_t RelaySideContextId, const xNetAddress & TargetAddress, const void * DataPtr, size_t DataSize) {
    if (xPP_PostUdpChannelData::MAX_PAYLOAD_SIZE < DataSize) {
        return;
    }
    auto PRC = GetRelayContextById(RelaySideContextId);
    if (!PRC) {
        return;
    }
    auto PDC = GetDeviceContextById(PRC->DeviceId);
    if (!PDC || !PDC->Handle.IsValid()) {
        return;
    }
    const char * PB         = static_cast<const char *>(DataPtr);
    auto         Req        = xPP_PostUdpChannelData();
    Req.TargetAddress       = TargetAddress;
    Req.DeviceSideContextId = PRC->DeviceSideContextId;
    Req.RelaySideContextId  = PRC->RelaySideContextId;
    Req.PayloadView         = { PB, DataSize };
    PDC->Handle.PostMessage(Cmd_DV_RL_PostUdpChannelData, 0, Req);
}

void NotifyProxyConnectionRefused(xRL_RelayContext * PRC) {
    auto N               = xPP_DeviceConnectionState();
    N.ProxySideContextId = PRC->ProxySideContextId;
    PostMessageToProxy(PRC->ProxyConnectionId, Cmd_PA_RL_NotifyConnectionState, 0, N);
}

void NotifyProxyConnectionEstablished(xRL_RelayContext * PRC) {
    auto N                = xPP_DeviceConnectionState();
    N.DeviceSideContextId = PRC->DeviceSideContextId;
    N.ProxySideContextId  = PRC->ProxySideContextId;
    N.RelaySideContextId  = PRC->RelaySideContextId;
    PostMessageToProxy(PRC->ProxyConnectionId, Cmd_PA_RL_NotifyConnectionState, 0, N);
}