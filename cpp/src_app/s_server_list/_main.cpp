#include "../lib_utils/all.hpp"
#include "./server_list_manager.hpp"

#include <pp_common/_.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

static auto RegisterServiceBindAddress = xNetAddress();
static auto DownloadServiceBindAddress = xNetAddress();
static auto BackendServerListFilename  = std::string();

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

struct xRegisterServerService : xService {

    void SetBackendServerListFile(const std::filesystem::path & FilePath) { ServerListManager.SetBackendServerListFile(FilePath); }

    void OnTick(uint64_t NowMS) override { ServerListManager.OnTick(NowMS); }

    void OnClientConnected(xServiceClientConnection & Connection) override { Logger->I("OnClientConnected"); }

    void OnClientClose(xServiceClientConnection & Connection) override {
        auto Type     = Connection.UserContext.U32;
        auto ServerId = Connection.UserContextEx.U64;
        if (!Type) {
            assert(!ServerId);
            return;
        }

        switch ((eServerType)Type) {
            case eServerType::AUTH_CACHE:
                Logger->I("RemoveAuthCacheServerInfo: ServerId=%" PRIi64 "", ServerId);
                ServerListManager.RemoveAuthCacheServerInfo(ServerId);
                break;
            case eServerType::AUDIT_DEVICE_CACHE:
                Logger->I("RemoveAuditDeviceServerInfo: ServerId=%" PRIi64 "", ServerId);
                ServerListManager.RemoveAuditDeviceServerInfo(ServerId);
                break;
            case eServerType::AUDIT_ACCOUNT_CACHE:
                Logger->I("RemoveAuditAccountServerInfo: ServerId=%" PRIi64 "", ServerId);
                ServerListManager.RemoveAuditAccountServerInfo(ServerId);
                break;
            case eServerType::DEVICE_STATE_RELAY:
                Logger->I("RemoveDeviceStateRelayServerInfo: ServerId=%" PRIi64 "", ServerId);
                ServerListManager.RemoveDeviceStateRelayServerInfo(ServerId);
                break;
            case eServerType::RELAY_INFO_DISPATCHER:
                Logger->I("RemoveRelayInfoDispatcherServerInfo: ServerId=%" PRIi64 "", ServerId);
                ServerListManager.CheckClearRelayInfoDispatcherServerInfo(ServerId);
                break;
            case eServerType::DEVICE_SELECTOR_DISPATCHER:
                Logger->I("RemoveDeviceSelectorDispatcherServerInfo: ServerId=%" PRIi64 "", ServerId);
                ServerListManager.RemoveDeviceSelectorDispatcherInfo(ServerId);
                break;
            default:
                Logger->F("Invalid server type detected");
                break;
        };
        return;
    }

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {

        Logger->I("OnClientPacket CommandId=%" PRIx32 "", CommandId);
        switch (CommandId) {
            case Cmd_RegisterAuthCacheServer:
                return OnRegisterAuthCacheServer(Connection, PayloadPtr, PayloadSize);
            case Cmd_RegisterAuditDeviceServer:
                return OnRegisterAuditDeviceServer(Connection, PayloadPtr, PayloadSize);
            case Cmd_RegisterAuditAccountServer:
                return OnRegisterAuditAccountServer(Connection, PayloadPtr, PayloadSize);
            case Cmd_RegisterDeviceStateRelayServer:
                return OnRegisterDeviceStateRelayServer(Connection, PayloadPtr, PayloadSize);
            case Cmd_RegisterRelayInfoDispatcherServer:
                return OnRegisterRelayInfoDispatcherServer(Connection, PayloadPtr, PayloadSize);
            case Cmd_RegisterDeviceSelectorDispatcherServer:
                return OnRegisterDeviceSelectorDispatcherServer(Connection, PayloadPtr, PayloadSize);
            default:
                break;
        }
        return true;
    }

    bool OnRegisterAuthCacheServer(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto & TypeRef     = Connection.UserContext.U32;
        auto & ServerIdRef = Connection.UserContextEx.U64;
        if (TypeRef) {
            assert(ServerIdRef);
            DEBUG_LOG("duplicated register server");
            return false;
        }
        auto R = xPP_RegisterAuthCacheServer();
        if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerId || !R.Address) {
            Logger->E("invalid request");
            return false;
        }
        if (!ServerListManager.AddAuthCacheServerInfo(R.ServerId, R.Address)) {
            Logger->E("failed to allocate server info");
            return false;
        }
        TypeRef     = (uint32_t)eServerType::AUTH_CACHE;
        ServerIdRef = R.ServerId;
        Logger->I("OnRegisterAuthCacheServer: ServerId=%" PRIi64 "", R.ServerId);
        return true;
    }

    bool OnRegisterAuditAccountServer(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto & TypeRef     = Connection.UserContext.U32;
        auto & ServerIdRef = Connection.UserContextEx.U64;
        if (TypeRef) {
            assert(ServerIdRef);
            DEBUG_LOG("duplicated register server");
            return false;
        }
        auto R = xPP_RegisterAuditAccountServer();
        if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerId || !R.Address) {
            Logger->E("invalid request");
            return false;
        }
        if (!ServerListManager.AddAuditAccountServerInfo(R.ServerId, R.Address)) {
            Logger->E("failed to allocate server info");
            return false;
        }
        TypeRef     = (uint32_t)eServerType::AUDIT_ACCOUNT_CACHE;
        ServerIdRef = R.ServerId;
        Logger->I("OnRegisterAuditAccountServer: ServerId=%" PRIi64 "", R.ServerId);
        return true;
    }

    bool OnRegisterAuditDeviceServer(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto & TypeRef     = Connection.UserContext.U32;
        auto & ServerIdRef = Connection.UserContextEx.U64;
        if (TypeRef) {
            assert(ServerIdRef);
            DEBUG_LOG("duplicated register server");
            return false;
        }
        auto R = xPP_RegisterAuditDeviceServer();
        if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerId || !R.Address) {
            Logger->E("invalid request");
            return false;
        }
        if (!ServerListManager.AddAuditDeviceServerInfo(R.ServerId, R.Address)) {
            Logger->E("failed to allocate server info");
            return false;
        }
        TypeRef     = (uint32_t)eServerType::AUDIT_DEVICE_CACHE;
        ServerIdRef = R.ServerId;
        Logger->I("OnRegisterAuditDeviceServer: ServerId=%" PRIi64 "", R.ServerId);
        return true;
    }

    bool OnRegisterDeviceStateRelayServer(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto & TypeRef     = Connection.UserContext.U32;
        auto & ServerIdRef = Connection.UserContextEx.U64;
        if (TypeRef) {
            assert(ServerIdRef);
            DEBUG_LOG("duplicated register server");
            return false;
        }

        auto R = xPP_RegisterDeviceStateRelayServer();
        if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerId || !R.Address || !R.ObserverAddress) {
            Logger->E("invalid request");
            return false;
        }
        if (!ServerListManager.AddDeviceStateRelayServerInfo(R.ServerId, R.Address, R.ObserverAddress)) {
            Logger->E("failed to allocate server info");
            return false;
        }
        TypeRef     = (uint32_t)eServerType::DEVICE_STATE_RELAY;
        ServerIdRef = R.ServerId;
        Logger->I("OnRegisterDeviceStateRelayServer: ServerId=%" PRIi64 "", R.ServerId);
        return true;
    }

    bool OnRegisterRelayInfoDispatcherServer(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto & TypeRef     = Connection.UserContext.U32;
        auto & ServerIdRef = Connection.UserContextEx.U64;
        if (TypeRef) {
            assert(ServerIdRef);
            DEBUG_LOG("duplicated register server");
            return false;
        }

        auto R = xPP_RegisterRelayInfoDispatcherServer();
        if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerInfo.ServerId || !R.ServerInfo.ProducerAddress || !R.ServerInfo.ObserverAddress) {
            Logger->E("invalid request");
            return false;
        }
        if (!ServerListManager.SetRelayInfoDispatcherServerInfo(R.ServerInfo)) {
            Logger->E("failed to allocate server info");
            return false;
        }
        TypeRef     = (uint32_t)eServerType::RELAY_INFO_DISPATCHER;
        ServerIdRef = R.ServerInfo.ServerId;
        Logger->I(
            "OnRegisterRelayInfoDispatcherServer: ServerId=%" PRIi64 ", ProducerAddress=%s, ObserverAddress=%s", R.ServerInfo.ServerId,
            R.ServerInfo.ProducerAddress.ToString().c_str(), R.ServerInfo.ObserverAddress.ToString().c_str()
        );
        return true;
    }

    bool OnRegisterDeviceSelectorDispatcherServer(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto & TypeRef     = Connection.UserContext.U32;
        auto & ServerIdRef = Connection.UserContextEx.U64;
        if (TypeRef) {
            assert(ServerIdRef);
            DEBUG_LOG("duplicated register server");
            return false;
        }

        auto R = xPP_RegisterDeviceSelectorDispatcher();
        if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerInfo.ServerId || !R.ServerInfo.ExportAddressForClient || !R.ServerInfo.ExportAddressForServiceProvider) {
            Logger->E("invalid request");
            return false;
        }
        if (!ServerListManager.AddDeviceSelectorDispatcherInfo(R.ServerInfo)) {
            Logger->E("failed to allocate server info");
            return false;
        }

        TypeRef     = (uint32_t)eServerType::DEVICE_SELECTOR_DISPATCHER;
        ServerIdRef = R.ServerInfo.ServerId;
        Logger->I("OnRegisterDeviceSelectorDispatcherServer: ServerId=%" PRIi64 "", R.ServerInfo.ServerId);

        return true;
    }

    auto & GetServerListManager() const { return ServerListManager; }

    //
private:
    xSL_InternalServerListManager ServerListManager;
};

xRegisterServerService RegisterServerService;

struct xDownloadServerService : xService {

    void OnClientConnected(xServiceClientConnection & Connection) override {}

    void OnClientClose(xServiceClientConnection & Connection) override {}

    bool OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
        switch (CommandId) {
            case Cmd_DownloadAuthCacheServerList:
                return OnDownloadAuthCacheServerList(Connection, PayloadPtr, PayloadSize);
            case Cmd_DownloadAuditDeviceServerList:
                return OnDownloadAuditDeviceServerList(Connection, PayloadPtr, PayloadSize);
            case Cmd_DownloadAuditAccountServerList:
                return OnDownloadAuditAccountServerList(Connection, PayloadPtr, PayloadSize);
            case Cmd_DownloadDeviceStateRelayServerList:
                return OnDownloadDeviceStateRelayServerList(Connection, PayloadPtr, PayloadSize);
            case Cmd_DownloadBackendServerList:
                return OnDownloadBackendServerList(Connection, PayloadPtr, PayloadSize);
            case Cmd_DownloadRelayInfoDispatcherServer:
                return OnDownloadRelayInfoDispatcherServer(Connection, PayloadPtr, PayloadSize);
            case Cmd_DownloadDeviceSelectorDispatcherServerList:
                return OnDownloadDeviceSelectorDispatcherList(Connection, PayloadPtr, PayloadSize);
            default:
                break;
        }

        return false;
    }

    //
    bool OnDownloadAuthCacheServerList(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {

        auto R = xPP_DownloadAuthCacheServerList();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            Logger->E("Invalid protocol");
            return false;
        }
        auto & M       = RegisterServerService.GetServerListManager();
        auto   Version = M.GetAuthCacheServerInfoListVersion();

        if (R.Version == Version) {
            auto Resp    = xPP_DownloadAuthCacheServerListResp();
            Resp.Version = Version;
            PostMessage(Connection, Cmd_DownloadAuthCacheServerListResp, 0, Resp);
            return true;
        }

        if (Version != AuthCacheServerListVersion) {
            auto List    = M.GetAuthCacheServerInfoList();
            auto Resp    = xPP_DownloadAuthCacheServerListResp();
            Resp.Version = Version;
            for (auto & S : List) {
                Resp.ServerInfoList.push_back({
                    .ServerId = S.ServerId,
                    .Address  = S.ServerAddress,
                });
            }
            AuthCacheServerListResponseSize = WriteMessage(AuthCacheServerListResponse, Cmd_DownloadAuthCacheServerListResp, 0, Resp);
            AuthCacheServerListVersion      = Version;
        }
        PostData(Connection, AuthCacheServerListResponse, AuthCacheServerListResponseSize);
        return true;
    }

    //
    bool OnDownloadAuditDeviceServerList(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto R = xPP_DownloadAuditDeviceServerList();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            return false;
        }
        auto & M       = RegisterServerService.GetServerListManager();
        auto   Version = M.GetAuditDeviceServerInfoListVersion();

        if (R.Version == Version) {
            auto Resp    = xPP_DownloadAuditDeviceServerListResp();
            Resp.Version = Version;
            PostMessage(Connection, Cmd_DownloadAuditDeviceServerListResp, 0, Resp);
            return true;
        }

        if (Version != AuditDeviceServerListVersion) {
            auto List    = M.GetAuditDeviceServerInfoList();
            auto Resp    = xPP_DownloadAuditDeviceServerListResp();
            Resp.Version = Version;
            for (auto & S : List) {
                Resp.ServerInfoList.push_back({
                    .ServerId = S.ServerId,
                    .Address  = S.ServerAddress,
                });
            }
            AuditDeviceServerListResponseSize = WriteMessage(AuditDeviceServerListResponse, Cmd_DownloadAuditDeviceServerListResp, 0, Resp);
            AuditDeviceServerListVersion      = Version;
        }
        PostData(Connection, AuditDeviceServerListResponse, AuditDeviceServerListResponseSize);
        return true;
    }

    //
    bool OnDownloadAuditAccountServerList(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto R = xPP_DownloadAuditAccountServerList();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            return false;
        }
        auto & M       = RegisterServerService.GetServerListManager();
        auto   Version = M.GetAuditAccountServerInfoListVersion();

        if (R.Version == Version) {
            auto Resp    = xPP_DownloadAuditAccountServerListResp();
            Resp.Version = Version;
            PostMessage(Connection, Cmd_DownloadAuditAccountServerListResp, 0, Resp);
            return true;
        }

        if (Version != AuditAccountServerListVersion) {
            auto List    = M.GetAuditAccountServerInfoList();
            auto Resp    = xPP_DownloadAuditAccountServerListResp();
            Resp.Version = Version;
            for (auto & S : List) {
                Resp.ServerInfoList.push_back({
                    .ServerId = S.ServerId,
                    .Address  = S.ServerAddress,
                });
            }
            AuditAccountServerListResponseSize = WriteMessage(AuditAccountServerListResponse, Cmd_DownloadAuditAccountServerListResp, 0, Resp);
            AuditAccountServerListVersion      = Version;
        }
        PostData(Connection, AuditAccountServerListResponse, AuditAccountServerListResponseSize);
        return true;
    }

    bool OnDownloadDeviceStateRelayServerList(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto R = xPP_DownloadDeviceStateRelayServerList();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            return false;
        }
        auto & M       = RegisterServerService.GetServerListManager();
        auto   Version = M.GetDeviceStateRelayServerInfoListVersion();

        if (R.Version == Version) {
            auto Resp    = xPP_DownloadDeviceStateRelayServerListResp();
            Resp.Version = Version;
            PostMessage(Connection, Cmd_DownloadDeviceStateRelayServerListResp, 0, Resp);
            return true;
        }

        if (Version != DeviceStateRelayServerListVersion) {
            auto List    = M.GetDeviceStateRelayServerInfoList();
            auto Resp    = xPP_DownloadDeviceStateRelayServerListResp();
            Resp.Version = Version;
            for (auto & S : List) {
                Resp.ServerInfoList.push_back({
                    .ServerId        = S.ServerId,
                    .ProducerAddress = S.ServerAddress,
                    .ObserverAddress = S.ObserverAddress,
                });
            }
            DeviceStateRelayServerListResponseSize = WriteMessage(DeviceStateRelayServerListResponse, Cmd_DownloadDeviceStateRelayServerListResp, 0, Resp);
            DeviceStateRelayServerListVersion      = Version;
        }
        PostData(Connection, DeviceStateRelayServerListResponse, DeviceStateRelayServerListResponseSize);
        return true;
    }

    bool OnDownloadBackendServerList(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto R = xPP_DownloadBackendServerList();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            Logger->E("Invalid protocol");
            return false;
        }
        auto & M       = RegisterServerService.GetServerListManager();
        auto   Version = M.GetBackendServerListVersion();

        if (R.Version == Version) {
            auto Resp    = xPP_DownloadBackendServerListResp();
            Resp.Version = Version;
            PostMessage(Connection, Cmd_DownloadBackendServerListResp, 0, Resp);
            return true;
        }

        if (Version != BackendServerListVersion) {
            auto List                     = M.GetBackendServerList();
            auto Resp                     = xPP_DownloadBackendServerListResp();
            Resp.Version                  = Version;
            Resp.ServerAddressList        = M.GetBackendServerList();
            BackendServerListResponseSize = WriteMessage(BackendServerListResponse, Cmd_DownloadBackendServerListResp, 0, Resp);
            BackendServerListVersion      = Version;
        }
        PostData(Connection, BackendServerListResponse, BackendServerListResponseSize);
        return true;
    }

    bool OnDownloadRelayInfoDispatcherServer(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {
        auto R = xPP_DownloadRelayInfoDispatcherServer();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            Logger->E("Invalid protocol");
            return false;
        }
        auto & M     = RegisterServerService.GetServerListManager();
        auto   PInfo = M.GetRelayInfoDispatcherServerInfo();

        auto Resp       = xPP_DownloadRelayInfoDispatcherServerResp();
        Resp.ServerInfo = *PInfo;

        PostMessage(Connection, Cmd_DownloadRelayInfoDispatcherServerResp, 0, Resp);
        return true;
    }

    bool OnDownloadDeviceSelectorDispatcherList(xServiceClientConnection & Connection, ubyte * PayloadPtr, size_t PayloadSize) {

        auto R = xPP_DownloadDeviceSelectorDispatcherList();
        if (!R.Deserialize(PayloadPtr, PayloadSize)) {
            Logger->E("Invalid protocol");
            return false;
        }
        auto & M       = RegisterServerService.GetServerListManager();
        auto   Version = M.GetDeviceSelectorDispatcherInfoListVersion();

        if (R.Version == Version) {
            auto Resp    = xPP_DownloadDeviceSelectorDispatcherListResp();
            Resp.Version = Version;
            PostMessage(Connection, Cmd_DownloadDeviceSelectorDispatcherServerListResp, 0, Resp);
            return true;
        }

        if (Version != DeviceSelectorDispatcherListVersion) {
            auto List    = M.GetDeviceSelectorDispatcherInfoList();
            auto Resp    = xPP_DownloadDeviceSelectorDispatcherListResp();
            Resp.Version = Version;
            for (auto & S : List) {
                Resp.ServerInfoList.push_back({
                    .ServerId                        = S.ServerId,
                    .ExportAddressForClient          = S.ExportAddressForClient,
                    .ExportAddressForServiceProvider = S.ExportAddressForServiceProvider,
                });
            }
            DeviceSelectorDispatcherListResponseSize = WriteMessage(DeviceSelectorDispatcherListResponse, Cmd_DownloadDeviceSelectorDispatcherServerListResp, 0, Resp);
            DeviceSelectorDispatcherListVersion      = Version;
        }
        PostData(Connection, DeviceSelectorDispatcherListResponse, DeviceSelectorDispatcherListResponseSize);
        return true;
    }

private:
    uint64_t AuthCacheServerListVersion = 0;
    ubyte    AuthCacheServerListResponse[MaxPacketSize];
    size_t   AuthCacheServerListResponseSize = {};

    uint64_t AuditDeviceServerListVersion = 0;
    ubyte    AuditDeviceServerListResponse[MaxPacketSize];
    size_t   AuditDeviceServerListResponseSize = {};

    uint64_t AuditAccountServerListVersion = 0;
    ubyte    AuditAccountServerListResponse[MaxPacketSize];
    size_t   AuditAccountServerListResponseSize = {};

    uint64_t DeviceStateRelayServerListVersion = 0;
    ubyte    DeviceStateRelayServerListResponse[MaxPacketSize];
    size_t   DeviceStateRelayServerListResponseSize = {};

    uint64_t BackendServerListVersion = 0;
    ubyte    BackendServerListResponse[MaxPacketSize];
    size_t   BackendServerListResponseSize = {};

    uint64_t DeviceSelectorDispatcherListVersion = 0;
    ubyte    DeviceSelectorDispatcherListResponse[MaxPacketSize];
    size_t   DeviceSelectorDispatcherListResponseSize = {};
};

xDownloadServerService DownloadServerService;

int main(int argc, char ** argv) {
    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Require(RegisterServiceBindAddress, "RegisterServiceBindAddress");
    CL.Require(DownloadServiceBindAddress, "DownloadServiceBindAddress");
    CL.Require(BackendServerListFilename, "BackendServerListFilename");

    auto RSSG = xResourceGuard(RegisterServerService, &IC, RegisterServiceBindAddress, 5000, true);
    auto DSSG = xResourceGuard(DownloadServerService, &IC, DownloadServiceBindAddress, 5000, true);

    RegisterServerService.SetBackendServerListFile(BackendServerListFilename);

    while (true) {
        ServiceTicker.Update();
        IC.LoopOnce();
        TickAll(ServiceTicker(), RegisterServerService, DownloadServerService);
    }

    return 0;
}
