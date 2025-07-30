#pragma once
#include <pp_common/_.hpp>
#include <pp_protocol/internal/all.hpp>

struct xSL_InternalServerInfo {
    uint64_t    ServerId;
    xNetAddress ServerAddress;
};

using xSL_AuthCacheServerInfo    = xSL_InternalServerInfo;
using xSL_AuditDeviceServerInfo  = xSL_InternalServerInfo;
using xSL_AuditAccountServerInfo = xSL_InternalServerInfo;

struct xSL_DeviceStateRelayServerInfo : xSL_InternalServerInfo {
    xNetAddress ObserverAddress;
};

class xSL_InternalServerListManager {
public:
    bool Init() {
        AuthCacheServerInfoListVersionTimestampMS    = Ticker();
        AuditDeviceServerInfoListVersionTimestampMS  = Ticker();
        AuditAccountServerInfoListVersionTimestampMS = Ticker();
        return true;
    };
    void Clean() {
        Reset(AuthCacheServerInfoListVersion);
        Reset(AuthCacheServerInfoListDirty);
        Reset(AuthCacheServerInfoListVersionTimestampMS);
        Reset(AuthCacheServerInfoList);
        Reset(VersionedAuthCacheServerInfoList);

        Reset(AuditDeviceServerInfoListVersion);
        Reset(AuditDeviceServerInfoListDirty);
        Reset(AuditDeviceServerInfoListVersionTimestampMS);
        Reset(AuditDeviceServerInfoList);
        Reset(VersionedAuditDeviceServerInfoList);

        Reset(AuditAccountServerInfoListVersion);
        Reset(AuditAccountServerInfoListDirty);
        Reset(AuditAccountServerInfoListVersionTimestampMS);
        Reset(AuditAccountServerInfoList);
        Reset(VersionedAuditAccountServerInfoList);

        Reset(DeviceStateRelayServerInfoListVersion);
        Reset(DeviceStateRelayServerInfoListDirty);
        Reset(DeviceStateRelayServerInfoListVersionTimestampMS);
        Reset(DeviceStateRelayServerInfoList);
        Reset(VersionedDeviceStateRelayServerInfoList);

        Reset(BackendServerListVersion);
        Reset(BackendServerListVersionTimestampMS);
        Reset(BackendServerList);
        Reset(BackendServerListFilePath);

        Reset(RelayInfoDispatcherServerInfo);
    }

    void OnTick(uint64_t NowMS);

    bool AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress);
    void RemoveAuthCacheServerInfo(uint64_t ServerId);
    auto GetAuthCacheServerInfo(uint64_t ServerId) -> const xSL_AuthCacheServerInfo *;
    auto GetAuthCacheServerInfoList() const -> const std::vector<xSL_AuthCacheServerInfo> & { return VersionedAuthCacheServerInfoList; }
    auto GetAuthCacheServerInfoListVersion() const { return AuthCacheServerInfoListVersion; }
    auto GetAuthCacheServerInfoListVersionTimestampMS() const { return AuthCacheServerInfoListVersionTimestampMS; }

    bool AddAuditDeviceServerInfo(uint64_t ServerId, xNetAddress ServerAddress);
    void RemoveAuditDeviceServerInfo(uint64_t ServerId);
    auto GetAuditDeviceServerInfo(uint64_t ServerId) -> const xSL_AuditDeviceServerInfo *;
    auto GetAuditDeviceServerInfoList() const -> const std::vector<xSL_AuditDeviceServerInfo> & { return VersionedAuditDeviceServerInfoList; }
    auto GetAuditDeviceServerInfoListVersion() const { return AuditDeviceServerInfoListVersion; }
    auto GetAuditDeviceServerInfoListVersionTimestampMS() const { return AuditDeviceServerInfoListVersionTimestampMS; }

    bool AddAuditAccountServerInfo(uint64_t ServerId, xNetAddress ServerAddress);
    void RemoveAuditAccountServerInfo(uint64_t ServerId);
    auto GetAuditAccountServerInfo(uint64_t ServerId) -> const xSL_AuditAccountServerInfo *;
    auto GetAuditAccountServerInfoList() const -> const std::vector<xSL_AuditAccountServerInfo> & { return VersionedAuditAccountServerInfoList; }
    auto GetAuditAccountServerInfoListVersion() const { return AuditAccountServerInfoListVersion; }
    auto GetAuditAccountServerInfoListVersionTimestampMS() const { return AuditAccountServerInfoListVersionTimestampMS; }

    // DSR
    bool AddDeviceStateRelayServerInfo(uint64_t ServerId, xNetAddress ServerAddress, xNetAddress ObserverAddress);
    void RemoveDeviceStateRelayServerInfo(uint64_t ServerId);
    auto GetDeviceStateRelayServerInfo(uint64_t ServerId) -> const xSL_DeviceStateRelayServerInfo *;
    auto GetDeviceStateRelayServerInfoList() const -> const std::vector<xSL_DeviceStateRelayServerInfo> & { return VersionedDeviceStateRelayServerInfoList; }
    auto GetDeviceStateRelayServerInfoListVersion() const { return DeviceStateRelayServerInfoListVersion; }
    auto GetDeviceStateRelayServerInfoListVersionTimestampMS() const { return DeviceStateRelayServerInfoListVersionTimestampMS; }

    // DSD
    bool AddDeviceSelectorDispatcherInfo(const xDeviceSelectorDispatcherInfo & ServerInfo);
    void RemoveDeviceSelectorDispatcherInfo(uint64_t ServerId);
    auto GetDeviceSelectorDispatcherInfo(uint64_t ServerId) -> const xDeviceSelectorDispatcherInfo *;
    auto GetDeviceSelectorDispatcherInfoList() const -> const std::vector<xDeviceSelectorDispatcherInfo> & { return VersionedDeviceSelectorDispatcherInfoList; }
    auto GetDeviceSelectorDispatcherInfoListVersion() const { return DeviceSelectorDispatcherInfoListVersion; }
    auto GetDeviceSelectorDispatcherInfoListVersionTimestampMS() const { return DeviceSelectorDispatcherInfoListVersionTimestampMS; }

    // BS
    void SetBackendServerListFile(const std::filesystem::path & FilePath) { this->BackendServerListFilePath = FilePath; }
    bool ReloadBackendServerList();  // return value: true new version of server list
    auto GetBackendServerList() const -> const std::vector<xNetAddress> & { return BackendServerList; }
    auto GetBackendServerListVersion() const { return BackendServerListVersion; }
    auto GetBackendServerListVersionTimestampMS() const { return BackendServerListVersionTimestampMS; }

    bool SetRelayInfoDispatcherServerInfo(const xRelayInfoDispatcherServerInfo & ServerInfo);
    auto GetRelayInfoDispatcherServerInfo() const -> const xRelayInfoDispatcherServerInfo * { return &RelayInfoDispatcherServerInfo; }
    void CheckClearRelayInfoDispatcherServerInfo(uint64_t ServerId) {
        assert(ServerId == RelayInfoDispatcherServerInfo.ServerId);
        Reset(RelayInfoDispatcherServerInfo);
    }

private:
    xTicker Ticker;

    uint32_t                             AuthCacheServerInfoListVersion            = 0;
    bool                                 AuthCacheServerInfoListDirty              = false;
    uint64_t                             AuthCacheServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AuthCacheServerInfo> AuthCacheServerInfoList;
    std::vector<xSL_AuthCacheServerInfo> VersionedAuthCacheServerInfoList;

    uint32_t                               AuditDeviceServerInfoListVersion            = 0;
    bool                                   AuditDeviceServerInfoListDirty              = false;
    uint64_t                               AuditDeviceServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AuditDeviceServerInfo> AuditDeviceServerInfoList;
    std::vector<xSL_AuditDeviceServerInfo> VersionedAuditDeviceServerInfoList;

    uint32_t                                AuditAccountServerInfoListVersion            = 0;
    bool                                    AuditAccountServerInfoListDirty              = false;
    uint64_t                                AuditAccountServerInfoListVersionTimestampMS = {};
    std::vector<xSL_AuditAccountServerInfo> AuditAccountServerInfoList;
    std::vector<xSL_AuditAccountServerInfo> VersionedAuditAccountServerInfoList;

    uint32_t                                    DeviceStateRelayServerInfoListVersion            = 0;
    bool                                        DeviceStateRelayServerInfoListDirty              = false;
    uint64_t                                    DeviceStateRelayServerInfoListVersionTimestampMS = {};
    std::vector<xSL_DeviceStateRelayServerInfo> DeviceStateRelayServerInfoList;
    std::vector<xSL_DeviceStateRelayServerInfo> VersionedDeviceStateRelayServerInfoList;

    uint32_t                                   DeviceSelectorDispatcherInfoListVersion            = 0;
    bool                                       DeviceSelectorDispatcherInfoListDirty              = false;
    uint64_t                                   DeviceSelectorDispatcherInfoListVersionTimestampMS = {};
    std::vector<xDeviceSelectorDispatcherInfo> DeviceSelectorDispatcherInfoList;
    std::vector<xDeviceSelectorDispatcherInfo> VersionedDeviceSelectorDispatcherInfoList;

    uint32_t                 BackendServerListVersion            = 0;
    uint64_t                 BackendServerListVersionTimestampMS = 0;
    std::vector<xNetAddress> BackendServerList;
    std::filesystem::path    BackendServerListFilePath;

    xRelayInfoDispatcherServerInfo RelayInfoDispatcherServerInfo;
};
