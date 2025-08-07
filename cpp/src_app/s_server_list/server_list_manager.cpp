#include "./server_list_manager.hpp"

#include "../lib_utils/all.hpp"

static constexpr const uint64_t UPGRADE_VERSION_TIMEOUT_MS           = 5'000;
static constexpr const uint64_t RELOAD_BACKEND_SERVER_LIST_TIMEOUTMS = 1 * 60'000;

void xSL_InternalServerListManager::OnTick(uint64_t NowMS) {
    Ticker.Update(NowMS);
    if (AuthCacheServerInfoListDirty && (NowMS - AuthCacheServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedAuthCacheServerInfoList");
        if (!++AuthCacheServerInfoListVersion) {
            ++AuthCacheServerInfoListVersion;
        }
        VersionedAuthCacheServerInfoList          = AuthCacheServerInfoList;
        AuthCacheServerInfoListVersionTimestampMS = NowMS;
        Reset(AuthCacheServerInfoListDirty);
    }

    if (AuditDeviceServerInfoListDirty && (NowMS - AuditDeviceServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedAuditDeviceServerInfoList");
        if (!++AuditDeviceServerInfoListVersion) {
            ++AuditDeviceServerInfoListVersion;
        }
        VersionedAuditDeviceServerInfoList          = AuditDeviceServerInfoList;
        AuditDeviceServerInfoListVersionTimestampMS = NowMS;
        Reset(AuditDeviceServerInfoListDirty);
    }

    if (AuditAccountServerInfoListDirty && (NowMS - AuditAccountServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedAuditAccountServerInfoList");
        if (!++AuditAccountServerInfoListVersion) {
            ++AuditAccountServerInfoListVersion;
        }
        VersionedAuditAccountServerInfoList          = AuditAccountServerInfoList;
        AuditAccountServerInfoListVersionTimestampMS = NowMS;
        Reset(AuditAccountServerInfoListDirty);
    }

    if (DeviceStateRelayServerInfoListDirty && (NowMS - DeviceStateRelayServerInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedDeviceStateRelayServerInfoListVersion");
        if (!++DeviceStateRelayServerInfoListVersion) {
            ++DeviceStateRelayServerInfoListVersion;
        }
        VersionedDeviceStateRelayServerInfoList          = DeviceStateRelayServerInfoList;
        DeviceStateRelayServerInfoListVersionTimestampMS = NowMS;
        Reset(DeviceStateRelayServerInfoListDirty);
    }

    if (DeviceSelectorDispatcherInfoListDirty && (NowMS - DeviceSelectorDispatcherInfoListVersionTimestampMS) > UPGRADE_VERSION_TIMEOUT_MS) {
        Logger->I("Update VersionedDeviceSelectorDispatcherInfoList");
        if (!++DeviceSelectorDispatcherInfoListVersion) {
            ++DeviceSelectorDispatcherInfoListVersion;
        }
        VersionedDeviceSelectorDispatcherInfoList          = DeviceSelectorDispatcherInfoList;
        DeviceSelectorDispatcherInfoListVersionTimestampMS = NowMS;
        Reset(DeviceSelectorDispatcherInfoListDirty);
    }

    if ((NowMS - BackendServerListVersionTimestampMS) >= RELOAD_BACKEND_SERVER_LIST_TIMEOUTMS) {
        BackendServerListVersionTimestampMS = NowMS;

        auto Updated = ReloadBackendServerList();
        if (Updated) {
            if (!++BackendServerListVersion) {
                ++BackendServerListVersion;
            }

            std::ostringstream OS;
            OS << "BackendServerList updated: from_file=" << BackendServerListFilePath << ", version=" << BackendServerListVersion << " size=" << BackendServerList.size() << endl;
            for (auto & A : BackendServerList) {
                OS << A.ToString() << endl;
            }
            OS << "BackendServerList finished here";
            Logger->I("%s", OS.str().c_str());
        }
    }
}

bool xSL_InternalServerListManager::AddAuthCacheServerInfo(uint64_t ServerId, xNetAddress ServerAddress) {
    if (AuthCacheServerInfoList.size() >= MAX_AUTH_CACHE_SERVER_COUNT) {
        return false;
    }

    auto It    = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    auto Found = (It != AuthCacheServerInfoList.end()) && (It->ServerId == ServerId);
    if (Found) {
        return false;
    }
    AuthCacheServerInfoList.emplace(
        It,
        xSL_AuthCacheServerInfo{
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    AuthCacheServerInfoListVersionTimestampMS = Ticker();
    AuthCacheServerInfoListDirty              = true;
    return true;
}

void xSL_InternalServerListManager::RemoveAuthCacheServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuthCacheServerInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    AuthCacheServerInfoList.erase(It);
    AuthCacheServerInfoListDirty = true;
}

auto xSL_InternalServerListManager::GetAuthCacheServerInfo(uint64_t ServerId) -> const xSL_AuthCacheServerInfo * {
    auto It = std::lower_bound(AuthCacheServerInfoList.begin(), AuthCacheServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuthCacheServerInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}

/////////////////////////////////////////////////

bool xSL_InternalServerListManager::AddAuditDeviceServerInfo(uint64_t ServerId, xNetAddress ServerAddress) {
    if (AuditDeviceServerInfoList.size() >= MAX_AUDIT_DEVICE_SERVER_COUNT) {
        return false;
    }

    auto It    = std::lower_bound(AuditDeviceServerInfoList.begin(), AuditDeviceServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    auto Found = (It != AuditDeviceServerInfoList.end()) && (It->ServerId == ServerId);
    if (Found) {
        return false;
    }
    AuditDeviceServerInfoList.emplace(
        It,
        xSL_AuditDeviceServerInfo{
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    AuditDeviceServerInfoListVersionTimestampMS = Ticker();
    AuditDeviceServerInfoListDirty              = true;
    return true;
}

void xSL_InternalServerListManager::RemoveAuditDeviceServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(AuditDeviceServerInfoList.begin(), AuditDeviceServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditDeviceServerInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    AuditDeviceServerInfoList.erase(It);
    AuditDeviceServerInfoListDirty = true;
}

auto xSL_InternalServerListManager::GetAuditDeviceServerInfo(uint64_t ServerId) -> const xSL_AuditDeviceServerInfo * {
    auto It = std::lower_bound(AuditDeviceServerInfoList.begin(), AuditDeviceServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditDeviceServerInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}

//////////////////////////

bool xSL_InternalServerListManager::AddAuditAccountServerInfo(uint64_t ServerId, xNetAddress ServerAddress) {
    if (AuditAccountServerInfoList.size() >= MAX_AUDIT_ACCOUNT_SERVER_COUNT) {
        return false;
    }

    auto It    = std::lower_bound(AuditAccountServerInfoList.begin(), AuditAccountServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    auto Found = (It != AuditAccountServerInfoList.end()) && (It->ServerId == ServerId);
    if (Found) {
        return false;
    }
    AuditAccountServerInfoList.emplace(
        It,
        xSL_AuditAccountServerInfo{
            .ServerId      = ServerId,
            .ServerAddress = ServerAddress,
        }
    );
    AuditAccountServerInfoListVersionTimestampMS = Ticker();
    AuditAccountServerInfoListDirty              = true;
    return true;
}

void xSL_InternalServerListManager::RemoveAuditAccountServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(AuditAccountServerInfoList.begin(), AuditAccountServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditAccountServerInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    AuditAccountServerInfoList.erase(It);
    AuditAccountServerInfoListDirty = true;
}

auto xSL_InternalServerListManager::GetAuditAccountServerInfo(uint64_t ServerId) -> const xSL_AuditAccountServerInfo * {
    auto It = std::lower_bound(AuditAccountServerInfoList.begin(), AuditAccountServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == AuditAccountServerInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}

bool xSL_InternalServerListManager::AddDeviceStateRelayServerInfo(uint64_t ServerId, xNetAddress ServerAddress, xNetAddress ObserverAddress) {
    if (AuditAccountServerInfoList.size() >= MAX_DEVICE_STATE_RELAY_SERVER_COUNT) {
        return false;
    }

    auto It = std::lower_bound(DeviceStateRelayServerInfoList.begin(), DeviceStateRelayServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    auto Found = (It != DeviceStateRelayServerInfoList.end()) && (It->ServerId == ServerId);
    if (Found) {
        return false;
    }
    auto New            = xSL_DeviceStateRelayServerInfo();
    New.ServerId        = ServerId;
    New.ServerAddress   = ServerAddress;
    New.ObserverAddress = ObserverAddress;

    DeviceStateRelayServerInfoList.insert(It, New);
    DeviceStateRelayServerInfoListVersionTimestampMS = Ticker();
    DeviceStateRelayServerInfoListDirty              = true;
    return true;
}

void xSL_InternalServerListManager::RemoveDeviceStateRelayServerInfo(uint64_t ServerId) {
    auto It = std::lower_bound(DeviceStateRelayServerInfoList.begin(), DeviceStateRelayServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == DeviceStateRelayServerInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    DeviceStateRelayServerInfoList.erase(It);
    DeviceStateRelayServerInfoListDirty = true;
}

auto xSL_InternalServerListManager::GetDeviceStateRelayServerInfo(uint64_t ServerId) -> const xSL_DeviceStateRelayServerInfo * {
    auto It = std::lower_bound(DeviceStateRelayServerInfoList.begin(), DeviceStateRelayServerInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == DeviceStateRelayServerInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}

// DSD

bool xSL_InternalServerListManager::AddDeviceSelectorDispatcherInfo(const xDeviceSelectorDispatcherInfo & ServerInfo) {
    if (DeviceSelectorDispatcherInfoList.size() >= MAX_DEVICE_SELECTOR_DISPATCHER_COUNT) {
        return false;
    }

    auto It    = std::lower_bound(DeviceSelectorDispatcherInfoList.begin(), DeviceSelectorDispatcherInfoList.end(), ServerInfo.ServerId, [](const auto & R, uint64_t Id) {
        return R.ServerId < Id;
    });
    auto Found = (It != DeviceSelectorDispatcherInfoList.end()) && (It->ServerId == ServerInfo.ServerId);
    if (Found) {
        return false;
    }
    DeviceSelectorDispatcherInfoList.emplace(It, ServerInfo);
    DeviceSelectorDispatcherInfoListVersionTimestampMS = Ticker();
    DeviceSelectorDispatcherInfoListDirty              = true;
    return true;
}

void xSL_InternalServerListManager::RemoveDeviceSelectorDispatcherInfo(uint64_t ServerId) {

    auto It =
        std::lower_bound(DeviceSelectorDispatcherInfoList.begin(), DeviceSelectorDispatcherInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == DeviceSelectorDispatcherInfoList.end()) {
        return;
    }
    if (It->ServerId != ServerId) {
        return;
    }
    DeviceSelectorDispatcherInfoList.erase(It);
    DeviceSelectorDispatcherInfoListDirty = true;
}

auto xSL_InternalServerListManager::GetDeviceSelectorDispatcherInfo(uint64_t ServerId) -> const xDeviceSelectorDispatcherInfo * {
    auto It =
        std::lower_bound(DeviceSelectorDispatcherInfoList.begin(), DeviceSelectorDispatcherInfoList.end(), ServerId, [](const auto & R, uint64_t Id) { return R.ServerId < Id; });
    if (It == DeviceSelectorDispatcherInfoList.end()) {
        return nullptr;
    }
    if (It->ServerId != ServerId) {
        return nullptr;
    }
    return &(*It);
}

//

bool xSL_InternalServerListManager::ReloadBackendServerList() {
    if (BackendServerListFilePath.empty()) {
        return false;
    }
    auto Segs        = FileToLines(BackendServerListFilePath);
    auto AddressList = std::vector<xNetAddress>();
    for (auto & S : Segs) {
        auto TS = Trim(S);
        if (TS.empty() || TS[0] == '#') {
            continue;
        }
        auto A = xNetAddress::Parse(TS);
        if (A && A.Port) {
            AddressList.push_back(A);
        }
    }
    std::sort(AddressList.begin(), AddressList.end());
    auto Last = std::unique(AddressList.begin(), AddressList.end());
    AddressList.erase(Last, AddressList.end());

    if (AddressList.size() != BackendServerList.size()) {
        BackendServerList = std::move(AddressList);
        return true;
    }
    for (size_t I = 0; I < AddressList.size(); ++I) {
        if (AddressList[I] != BackendServerList[I]) {
            BackendServerList = std::move(AddressList);
            return true;
        }
    }

    return false;
}

bool xSL_InternalServerListManager::SetRelayInfoDispatcherServerInfo(const xRelayInfoDispatcherServerInfo & ServerInfo) {
    if (RelayInfoDispatcherServerInfo.ServerId && ServerInfo.ServerId) {
        Logger->E(
            "Duplicated RelayInfoDispaterRegistered, force updating server info, previous server info: ServerId=%" PRIx64 ", ProducerAddress=%s, ObserverAddress=%s",
            RelayInfoDispatcherServerInfo.ServerId, RelayInfoDispatcherServerInfo.ProducerAddress.ToString().c_str(),
            RelayInfoDispatcherServerInfo.ObserverAddress.ToString().c_str()
        );
        return false;
    }

    RelayInfoDispatcherServerInfo = ServerInfo;
    Logger->I(
        "New RelayInfoDispaterRegistered, ServerId=%" PRIx64 ", ProducerAddress=%s, ObserverAddress=%s",  //
        RelayInfoDispatcherServerInfo.ServerId,                                                           //
        RelayInfoDispatcherServerInfo.ProducerAddress.ToString().c_str(),                                 //
        RelayInfoDispatcherServerInfo.ObserverAddress.ToString().c_str()
    );
    return true;
}
