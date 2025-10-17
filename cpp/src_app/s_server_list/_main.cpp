
#include "./_main.hpp"

#include <pp_common/_.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

static auto RegisterServiceBindAddress4 = xNetAddress();
static auto DownloadServiceBindAddress4 = xNetAddress();
static auto BackendServerListFilename   = std::string();

//////////
static uint64_t AuthCacheServerListVersion = 0;
static ubyte    AuthCacheServerListResponse[MaxPacketSize];
static size_t   AuthCacheServerListResponseSize = {};

static uint64_t AuditAccountServerListVersion = 0;
static ubyte    AuditAccountServerListResponse[MaxPacketSize];
static size_t   AuditAccountServerListResponseSize = {};

static uint64_t AuditDeviceServerListVersion = 0;
static ubyte    AuditDeviceServerListResponse[MaxPacketSize];
static size_t   AuditDeviceServerListResponseSize = {};

static uint64_t AuditTargetServerListVersion = 0;
static ubyte    AuditTargetServerListResponse[MaxPacketSize];
static size_t   AuditTargetServerListResponseSize = {};

static uint64_t DeviceStateRelayServerListVersion = 0;
static ubyte    DeviceStateRelayServerListResponse[MaxPacketSize];
static size_t   DeviceStateRelayServerListResponseSize = {};

static uint64_t BackendServerListVersion = 0;
static ubyte    BackendServerListResponse[MaxPacketSize];
static size_t   BackendServerListResponseSize = {};

static uint64_t DeviceSelectorDispatcherListVersion = 0;
static ubyte    DeviceSelectorDispatcherListResponse[MaxPacketSize];
static size_t   DeviceSelectorDispatcherListResponseSize = {};

///////////
static xTcpService RegisterService4;
static xTcpService DownloadService4;

static xSL_InternalServerListManager ServerListManager;

void OnRegisterClientClose(const xTcpServiceClientConnectionHandle & Handle) {
    auto Type     = Handle->UserContext.U32;
    auto ServerId = Handle->UserContextEx.U64;
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
        case eServerType::AUDIT_TARGET_CACHE:
            Logger->I("RemoveAuditTargetServerInfo: ServerId=%" PRIi64 "", ServerId);
            ServerListManager.RemoveAuditTargetServerInfo(ServerId);
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

bool OnRegisterClientPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    Logger->I("OnClientPacket CommandId=%" PRIx32 "", CommandId);
    switch (CommandId) {
        case Cmd_RegisterAuthCacheServer:
            return OnRegisterAuthCacheServer(Handle, PayloadPtr, PayloadSize);
        case Cmd_RegisterAuditDeviceServer:
            return OnRegisterAuditDeviceServer(Handle, PayloadPtr, PayloadSize);
        case Cmd_RegisterAuditAccountServer:
            return OnRegisterAuditAccountServer(Handle, PayloadPtr, PayloadSize);
        case Cmd_RegisterAuditTargetServer:
            return OnRegisterAuditTargetServer(Handle, PayloadPtr, PayloadSize);
        case Cmd_RegisterDeviceStateRelayServer:
            return OnRegisterDeviceStateRelayServer(Handle, PayloadPtr, PayloadSize);
        case Cmd_RegisterRelayInfoDispatcherServer:
            return OnRegisterRelayInfoDispatcherServer(Handle, PayloadPtr, PayloadSize);
        case Cmd_RegisterDeviceSelectorDispatcherServer:
            return OnRegisterDeviceSelectorDispatcherServer(Handle, PayloadPtr, PayloadSize);
        default:
            break;
    }
    return true;
}

bool OnRegisterAuthCacheServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto & TypeRef     = Handle->UserContext.U32;
    auto & ServerIdRef = Handle->UserContextEx.U64;
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

bool OnRegisterAuditAccountServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto & TypeRef     = Handle->UserContext.U32;
    auto & ServerIdRef = Handle->UserContextEx.U64;
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

bool OnRegisterAuditDeviceServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto & TypeRef     = Handle->UserContext.U32;
    auto & ServerIdRef = Handle->UserContextEx.U64;
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

bool OnRegisterAuditTargetServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {

    auto & TypeRef     = Handle->UserContext.U32;
    auto & ServerIdRef = Handle->UserContextEx.U64;
    if (TypeRef) {
        assert(ServerIdRef);
        DEBUG_LOG("duplicated register server");
        return false;
    }
    auto R = xPP_RegisterAuditTargetServer();
    if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerId || !R.Address) {
        Logger->E("invalid request");
        return false;
    }
    if (!ServerListManager.AddAuditTargetServerInfo(R.ServerId, R.Address)) {
        Logger->E("failed to allocate server info");
        return false;
    }
    TypeRef     = (uint32_t)eServerType::AUDIT_TARGET_CACHE;
    ServerIdRef = R.ServerId;
    Logger->I("OnRegisterAuditTargetServer: ServerId=%" PRIi64 ", Address=%s", R.ServerId, R.Address.ToString().c_str());
    return true;
}

bool OnRegisterDeviceStateRelayServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto & TypeRef     = Handle->UserContext.U32;
    auto & ServerIdRef = Handle->UserContextEx.U64;
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

bool OnRegisterRelayInfoDispatcherServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto & TypeRef     = Handle->UserContext.U32;
    auto & ServerIdRef = Handle->UserContextEx.U64;
    if (TypeRef) {
        assert(ServerIdRef);
        DEBUG_LOG("duplicated register server");
        return false;
    }

    auto R = xPP_RegisterRelayInfoDispatcherServer();
    if (!R.Deserialize(PayloadPtr, PayloadSize) || !R.ServerInfo.ServerId) {
        Logger->E("invalid request");
        return false;
    }
    if (!ServerListManager.SetRelayInfoDispatcherServerInfo(R.ServerInfo)) {
        Logger->E("failed to allocate server info");
        return false;
    }
    TypeRef     = (uint32_t)eServerType::RELAY_INFO_DISPATCHER;
    ServerIdRef = R.ServerInfo.ServerId;
    Logger->I("OnRegisterRelayInfoDispatcherServer: %s", R.ServerInfo.ToString().c_str());
    return true;
}

bool OnRegisterDeviceSelectorDispatcherServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto & TypeRef     = Handle->UserContext.U32;
    auto & ServerIdRef = Handle->UserContextEx.U64;
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

bool OnDownloadClientPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_DownloadAuthCacheServerList:
            return OnDownloadAuthCacheServerList(Handle, PayloadPtr, PayloadSize);
        case Cmd_DownloadAuditDeviceServerList:
            return OnDownloadAuditDeviceServerList(Handle, PayloadPtr, PayloadSize);
        case Cmd_DownloadAuditAccountServerList:
            return OnDownloadAuditAccountServerList(Handle, PayloadPtr, PayloadSize);
        case Cmd_DownloadAuditTargetServerList:
            return OnDownloadAuditTargetServerList(Handle, PayloadPtr, PayloadSize);
        case Cmd_DownloadDeviceStateRelayServerList:
            return OnDownloadDeviceStateRelayServerList(Handle, PayloadPtr, PayloadSize);
        case Cmd_DownloadBackendServerList:
            return OnDownloadBackendServerList(Handle, PayloadPtr, PayloadSize);
        case Cmd_DownloadRelayInfoDispatcherServer:
            return OnDownloadRelayInfoDispatcherServer(Handle, PayloadPtr, PayloadSize);
        case Cmd_DownloadDeviceSelectorDispatcherServerList:
            return OnDownloadDeviceSelectorDispatcherList(Handle, PayloadPtr, PayloadSize);
        default:
            break;
    }
    return false;
}

bool OnDownloadAuthCacheServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadAuthCacheServerList();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("Invalid protocol");
        return false;
    }
    auto & M       = ServerListManager;
    auto   Version = M.GetAuthCacheServerInfoListVersion();

    if (R.Version == Version) {
        auto Resp    = xPP_DownloadAuthCacheServerListResp();
        Resp.Version = Version;
        Handle.PostMessage(Cmd_DownloadAuthCacheServerListResp, 0, Resp);
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
    Handle.PostData(AuthCacheServerListResponse, AuthCacheServerListResponseSize);
    return true;
}

bool OnDownloadAuditDeviceServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadAuditDeviceServerList();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    auto & M       = ServerListManager;
    auto   Version = M.GetAuditDeviceServerInfoListVersion();

    if (R.Version == Version) {
        auto Resp    = xPP_DownloadAuditDeviceServerListResp();
        Resp.Version = Version;
        Handle.PostMessage(Cmd_DownloadAuditDeviceServerListResp, 0, Resp);
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
    Handle.PostData(AuditDeviceServerListResponse, AuditDeviceServerListResponseSize);
    return true;
}

bool OnDownloadAuditAccountServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadAuditAccountServerList();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    auto & M       = ServerListManager;
    auto   Version = M.GetAuditAccountServerInfoListVersion();

    if (R.Version == Version) {
        auto Resp    = xPP_DownloadAuditAccountServerListResp();
        Resp.Version = Version;
        Handle.PostMessage(Cmd_DownloadAuditAccountServerListResp, 0, Resp);
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
    Handle.PostData(AuditAccountServerListResponse, AuditAccountServerListResponseSize);
    return true;
}

bool OnDownloadAuditTargetServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadAuditTargetServerList();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    auto & M       = ServerListManager;
    auto   Version = M.GetAuditTargetServerInfoListVersion();

    if (R.Version == Version) {
        auto Resp    = xPP_DownloadAuditTargetServerListResp();
        Resp.Version = Version;
        Handle.PostMessage(Cmd_DownloadAuditTargetServerListResp, 0, Resp);
        return true;
    }

    if (Version != AuditTargetServerListVersion) {
        auto List    = M.GetAuditTargetServerInfoList();
        auto Resp    = xPP_DownloadAuditTargetServerListResp();
        Resp.Version = Version;
        for (auto & S : List) {
            Resp.ServerInfoList.push_back({
                .ServerId = S.ServerId,
                .Address  = S.ServerAddress,
            });
        }
        AuditTargetServerListResponseSize = WriteMessage(AuditTargetServerListResponse, Cmd_DownloadAuditTargetServerListResp, 0, Resp);
        AuditTargetServerListVersion      = Version;
    }
    Handle.PostData(AuditTargetServerListResponse, AuditTargetServerListResponseSize);
    return true;
}

bool OnDownloadDeviceStateRelayServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadDeviceStateRelayServerList();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        return false;
    }
    auto & M       = ServerListManager;
    auto   Version = M.GetDeviceStateRelayServerInfoListVersion();

    if (R.Version == Version) {
        auto Resp    = xPP_DownloadDeviceStateRelayServerListResp();
        Resp.Version = Version;
        Handle.PostMessage(Cmd_DownloadDeviceStateRelayServerListResp, 0, Resp);
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
    Handle.PostData(DeviceStateRelayServerListResponse, DeviceStateRelayServerListResponseSize);
    return true;
}

bool OnDownloadBackendServerList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadBackendServerList();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("Invalid protocol");
        return false;
    }
    auto & M       = ServerListManager;
    auto   Version = M.GetBackendServerListVersion();

    if (R.Version == Version) {
        auto Resp    = xPP_DownloadBackendServerListResp();
        Resp.Version = Version;
        Handle.PostMessage(Cmd_DownloadBackendServerListResp, 0, Resp);
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
    Handle.PostData(BackendServerListResponse, BackendServerListResponseSize);
    return true;
}

bool OnDownloadRelayInfoDispatcherServer(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadRelayInfoDispatcherServer();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("Invalid protocol");
        return false;
    }
    auto & M     = ServerListManager;
    auto   PInfo = M.GetRelayInfoDispatcherServerInfo();

    auto Resp       = xPP_DownloadRelayInfoDispatcherServerResp();
    Resp.ServerInfo = *PInfo;

    Handle.PostMessage(Cmd_DownloadRelayInfoDispatcherServerResp, 0, Resp);
    return true;
}

bool OnDownloadDeviceSelectorDispatcherList(const xTcpServiceClientConnectionHandle & Handle, ubyte * PayloadPtr, size_t PayloadSize) {
    auto R = xPP_DownloadDeviceSelectorDispatcherList();
    if (!R.Deserialize(PayloadPtr, PayloadSize)) {
        Logger->E("Invalid protocol");
        return false;
    }
    auto & M       = ServerListManager;
    auto   Version = M.GetDeviceSelectorDispatcherInfoListVersion();

    if (R.Version == Version) {
        auto Resp    = xPP_DownloadDeviceSelectorDispatcherListResp();
        Resp.Version = Version;
        Handle.PostMessage(Cmd_DownloadDeviceSelectorDispatcherServerListResp, 0, Resp);
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
    Handle.PostData(DeviceSelectorDispatcherListResponse, DeviceSelectorDispatcherListResponseSize);
    return true;
}

int main(int argc, char ** argv) {
    auto SEG = xRuntimeEnvGuard(argc, argv);
    auto CL  = xConfigLoader(RuntimeEnv.DefaultConfigFilePath);
    CL.Optional(RegisterServiceBindAddress4, "RegisterServiceBindAddress4");
    CL.Optional(DownloadServiceBindAddress4, "DownloadServiceBindAddress4");
    CL.Optional(BackendServerListFilename, "BackendServerListFilename");

    bool V4Enabled = RegisterServiceBindAddress4.Is4() && RegisterServiceBindAddress4.Port && DownloadServiceBindAddress4.Is4() && DownloadServiceBindAddress4.Port;
    if (!V4Enabled) {
        Logger->F("invalid config, v4 is disabled");
        return 0;
    }

    X_COND_GUARD(V4Enabled, RegisterService4, ServiceIoContext, RegisterServiceBindAddress4, 10'0000);
    X_COND_GUARD(V4Enabled, DownloadService4, ServiceIoContext, DownloadServiceBindAddress4, 10'0000);

    RegisterService4.OnClientClose  = OnRegisterClientClose;
    RegisterService4.OnClientPacket = OnRegisterClientPacket;
    DownloadService4.OnClientPacket = OnDownloadClientPacket;

    ServerListManager.SetBackendServerListFile(BackendServerListFilename);

    while (true) {
        ServiceUpdateOnce(ServerListManager, RegisterService4, DownloadService4);
    }

    return 0;
}
