#include "./device_selector.hpp"

#include "./_global.hpp"

#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

bool xDS_DeviceSelectorServiceProvider::Init(xIoContext * ICP) {
    RuntimeAssert(ClientPool.Init(ICP));
    ClientPool.SetOnConnectedCallback([this](xMessagePoster * Poster) { RegisterServiceProvider(Poster); });
    ClientPool.SetOnPacketCallback([this](xMessagePoster * Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) -> bool {
        // TODO: process message
        return true;
    });

    return true;
}

void xDS_DeviceSelectorServiceProvider::Tick(uint64_t NowMS) {
    TickAll(NowMS, ClientPool);
}

void xDS_DeviceSelectorServiceProvider::Clean() {
    ClientPool.Clean();
}

void xDS_DeviceSelectorServiceProvider::RegisterServiceProvider(xMessagePoster * Poster) {
    auto R = xPP_RegisterDeviceSelector();
    // TODO: set server info

    Poster->PostMessage(Cmd_RegisterDeviceSelector, 0, R);
}

// void xDS_DeviceSelectorService::OnClientClose(xServiceClientConnection & Connection) {
// }

// bool xDS_DeviceSelectorService::OnClientPacket(
//     xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
// ) {
//     DEBUG_LOG("CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());

//     switch (CommandId) {
//         case Cmd_DeviceSelector_AcquireDevice:
//             return OnSelectDevice(Connection, RequestId, PayloadPtr, PayloadSize);
//         default:
//             DEBUG_LOG("Invalid command id");
//             return true;
//     }
//     return true;
// }

// void xDS_DeviceSelectorService::OnCleanupClientConnection(const xServiceClientConnection & Connection) {
// }

// bool xDS_DeviceSelectorService::OnSelectDevice(xServiceClientConnection & CC, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
//     auto Req = xPP_AcquireDevice();
//     if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
//         DEBUG_LOG("Invalid protocol");
//         return true;
//     }
//     DEBUG_LOG("RequestGeoInfo: %u/%u/%u", (unsigned)Req.CountryId, (unsigned)Req.StateId, (unsigned)Req.CityId);

//     auto PD = (const xDS_DeviceContext *)nullptr;
//     if (Req.CityId) {
//         DEBUG_LOG("by CityId");
//         PD = DeviceContextManager.SelectDeviceByCityId(Req.CityId);
//     } else if (Req.StateId) {
//         DEBUG_LOG("by StateId");
//         PD = DeviceContextManager.SelectDeviceByStateId(Req.StateId);
//     } else if (Req.CountryId) {
//         DEBUG_LOG("by CountryId");
//         PD = DeviceContextManager.SelectDeviceByCountryId(Req.CountryId);
//     } else {
//         DEBUG_LOG("no device select condition");
//     }

//     auto Resp = xPP_AcquireDeviceResp();
//     if (PD) {
//         Resp.DeviceRelayServerRuntimeId = PD->InfoBase.ReleayServerRuntimeId;
//         Resp.DeviceRelaySideId          = PD->InfoBase.RelaySideDeviceId;
//         DEBUG_LOG("DeviceSelected: ServerId=%" PRIx64 ", DeviceId=%" PRIx64 "", Resp.DeviceRelayServerRuntimeId, Resp.DeviceRelaySideId);
//     } else {
//         DEBUG_LOG("No device found!");
//     }

//     PostMessage(CC, Cmd_DeviceSelector_AcquireDeviceResp, RequestId, Resp);
//     return true;
// }

void xDS_DeviceSelectorServiceProvider::UpdateDispatcherList(const std::vector<xDeviceSelectorDispatcherInfo> & ServerList) {
    auto L = std::vector<xNetAddress>();
    for (auto & S : ServerList) {
        L.push_back(S.ExportAddressForServiceProvider);
    }
    ClientPool.UpdateServerList(L);
}

/***
 *
 *
 *
 *
 */

bool xDS_DeviceObserver::OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DSR_DS_DeviceUpdate: {
            DEBUG_LOG("");
            auto PP = xPP_DeviceInfoUpdate();
            if (!PP.Deserialize(PayloadPtr, PayloadSize)) {
                DEBUG_LOG("Invalid device info");
                return true;
            }

            if (!PP.IsOffline) {
                auto LocalDevInfo                  = xDR_DeviceInfoBase{};
                LocalDevInfo.DeviceId              = PP.DeviceUuid;
                LocalDevInfo.ReleayServerRuntimeId = PP.RelayServerRuntimeId;
                LocalDevInfo.RelaySideDeviceId     = PP.RelaySideDeviceKey;

                LocalDevInfo.CountryId = PP.CountryId;
                LocalDevInfo.StateId   = PP.StateId;
                LocalDevInfo.CityId    = PP.CityId;

                DeviceContextManager.UpdateDevice(LocalDevInfo);
            } else {
                DeviceContextManager.RemoveDeviceById(PP.DeviceUuid);
            }
            break;
        }

        default:
            break;
    }
    DEBUG_LOG("CommandId: %" PRIx64 ", \n%s", CommandId, HexShow(PayloadPtr, PayloadSize).c_str());
    return true;
}
