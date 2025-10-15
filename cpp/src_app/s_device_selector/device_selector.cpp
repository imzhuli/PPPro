#include "./device_selector.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

bool xDS_DeviceSelectorServiceProvider::Init(xIoContext * ICP) {
    RuntimeAssert(ClientPool.Init(ICP));
    ClientPool.OnConnectedCallback = [this](const xMessageChannel & Poster) { RegisterServiceProvider(Poster); };
    ClientPool.OnPacketCallback = [this](const xMessageChannel & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) -> bool {
        switch (CommandId) {
            case Cmd_RegisterServerResp:
                return OnRegisterServerResp(PayloadPtr, PayloadSize);
            case Cmd_DeviceSelector_AcquireDevice:
                return OnSelectDevice(Source, RequestId, PayloadPtr, PayloadSize);
            default:
                DEBUG_LOG("Invalid command id");
                return false;
        }
        return true;
    };

    return true;
}

void xDS_DeviceSelectorServiceProvider::Tick(uint64_t NowMS) {
    TickAll(NowMS, ClientPool);
}

void xDS_DeviceSelectorServiceProvider::Clean() {
    ClientPool.Clean();
}

bool xDS_DeviceSelectorServiceProvider::OnRegisterServerResp(ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_RegisterServerResp();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }
    if (!R.Accepted) {
        DEBUG_LOG("not accepted");
        return false;
    }
    DEBUG_LOG("device selector accepted");
    return true;
}

void xDS_DeviceSelectorServiceProvider::RegisterServiceProvider(const xMessageChannel & Poster) {
    auto   R                = xPP_RegisterDeviceSelector();
    auto & ServerInfo       = R.ServerInfo;
    ServerInfo.DevicePoolId = DevicePoolId;
    if (EnableGenericV4Device) {
        ServerInfo.StrategyFlags |= DSS_IPV4;
    }
    if (EnableGenericV6Device) {
        ServerInfo.StrategyFlags |= DSS_IPV6;
    }
    if (EnablePersistentDeviceBinding) {
        ServerInfo.StrategyFlags |= DSS_DEVICE_PERSISTENT;
    }

    Poster.PostMessage(Cmd_RegisterDeviceSelector, 0, R);
}

bool xDS_DeviceSelectorServiceProvider::OnSelectDevice(const xMessageChannel & Source, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    auto Req = xPP_AcquireDevice();
    if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return false;
    }

    // TODO: select device:
    DEBUG_LOG("RequestGeoInfo: %u/%u/%u", (unsigned)Req.CountryId, (unsigned)Req.StateId, (unsigned)Req.CityId);
    auto PD = (const xDS_DeviceContext *)nullptr;
    if (Req.CityId) {
        DEBUG_LOG("by CityId");
        PD = DeviceContextManager.SelectDeviceByCityId(Req.CityId);
    } else if (Req.StateId) {
        DEBUG_LOG("by StateId");
        PD = DeviceContextManager.SelectDeviceByStateId(Req.StateId);
    } else if (Req.CountryId) {
        DEBUG_LOG("by CountryId");
        PD = DeviceContextManager.SelectDeviceByCountryId(Req.CountryId);
    } else {
        PD = DeviceContextManager.SelectDeviceGlobal();
    }

    auto Resp = xPP_AcquireDeviceResp();
    if (PD) {
        Resp.DeviceRelayServerRuntimeId = PD->InfoBase.ReleayServerRuntimeId;
        Resp.DeviceRelaySideId          = PD->InfoBase.RelayServerSideDeviceId;
        DEBUG_LOG("DeviceSelected: ServerId=%" PRIx64 ", DeviceId=%" PRIx64 "", Resp.DeviceRelayServerRuntimeId, Resp.DeviceRelaySideId);
    } else {
        DEBUG_LOG("No device found!");
    }

    Source.PostMessage(Cmd_DeviceSelector_AcquireDeviceResp, RequestId, Resp);
    return true;
}

void xDS_DeviceSelectorServiceProvider::UpdateDispatcherList(const std::vector<xDeviceSelectorDispatcherInfo> & ServerList) {
    auto L = std::vector<xNetAddress>();
    for (auto & S : ServerList) {
        L.push_back(S.ExportAddressForServiceProvider);
    }
    ClientPool.UpdateServerList(L);
}
