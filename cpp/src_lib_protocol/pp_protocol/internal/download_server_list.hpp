#pragma once
#include <pp_common/_.hpp>

/** !!!! IMPORTANT:
 *
 *  When download client reconnected to the server_list service,
 *  the FIRST request VERSION, should always be 0, in case server restarts or reset version numver
 *
 * */

struct xPP_DownloadAuthCacheServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadAuthCacheServerListResp : xBinaryMessage {

    void SerializeMembers() override {
        W(Version);

        uint32_t Count = ServerInfoList.size();
        W(Count);
        for (auto & I : ServerInfoList) {
            W(I.ServerId);
            W(I.Address);
        }
    }

    void DeserializeMembers() override {
        R(Version);

        uint32_t Count = 0;
        R(Count);

        if (Count >= MAX_AUTH_CACHE_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.Address);
        }
    }

    uint64_t                 Version;
    std::vector<xServerInfo> ServerInfoList;
    //
};

struct xPP_DownloadAuditDeviceServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadAuditDeviceServerListResp : xBinaryMessage {

    void SerializeMembers() override {
        W(Version);

        uint32_t Count = ServerInfoList.size();
        W(Count);
        for (auto & I : ServerInfoList) {
            W(I.ServerId);
            W(I.Address);
        }
    }

    void DeserializeMembers() override {
        R(Version);

        uint32_t Count = 0;
        R(Count);

        if (Count >= MAX_AUDIT_DEVICE_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.Address);
        }
    }

    uint32_t                 Version;
    std::vector<xServerInfo> ServerInfoList;
    //
};

struct xPP_DownloadAuditAccountServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadAuditAccountServerListResp : xBinaryMessage {

    void SerializeMembers() override {
        W(Version);
        uint32_t Count = ServerInfoList.size();
        W(Count);
        for (auto & I : ServerInfoList) {
            W(I.ServerId);
            W(I.Address);
        }
    }

    void DeserializeMembers() override {
        R(Version);

        uint32_t Count = 0;
        R(Count);

        if (Count >= MAX_AUDIT_ACCOUNT_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.Address);
        }
    }

    uint32_t                 Version;
    std::vector<xServerInfo> ServerInfoList;
    //
};

//// 3

struct xPP_DownloadDeviceStateRelayServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadDeviceStateRelayServerListResp : xBinaryMessage {

    void SerializeMembers() override {
        W(Version);
        uint32_t Count = ServerInfoList.size();
        W(Count);
        for (auto & I : ServerInfoList) {
            W(I.ServerId);
            W(I.ProducerAddress);
            W(I.ObserverAddress);
        }
    }

    void DeserializeMembers() override {
        R(Version);

        uint32_t Count = 0;
        R(Count);

        if (Count >= MAX_DEVICE_STATE_RELAY_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.ProducerAddress);
            R(I.ObserverAddress);
        }
    }

    uint32_t                                 Version;
    std::vector<xDeviceStateRelayServerInfo> ServerInfoList;
    //
};

// 4

struct xPP_DownloadBackendServerList : xBinaryMessage {
    void     SerializeMembers() override { W(Version); }
    void     DeserializeMembers() override { R(Version); }
    uint32_t Version;
};

struct xPP_DownloadBackendServerListResp : xBinaryMessage {

    void SerializeMembers() override {
        auto Count = (uint32_t)ServerAddressList.size();
        W(Version);
        W(Count);
        for (auto & A : ServerAddressList) {
            W(A);
        }
    }

    void DeserializeMembers() override {
        auto Count = (uint32_t)0;
        R(Version);
        R(Count);

        if (Count >= MAX_BACKEND_SERVER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerAddressList.resize(Count);
        for (auto & A : ServerAddressList) {
            R(A);
        }
    }

    uint32_t                 Version;
    std::vector<xNetAddress> ServerAddressList;
    //
};

struct xPP_DownloadRelayInfoDispatcherServer : xBinaryMessage {

    void SerializeMembers() override { W(Magic); }

    void DeserializeMembers() override {
        uint32_t Check = 0;
        R(Check);
        if (Check != Magic) {
            GetReader()->SetError();
        }
    }

    static constexpr const uint32_t Magic = 0xCD;
};

struct xPP_DownloadRelayInfoDispatcherServerResp : xBinaryMessage {

    void SerializeMembers() override {  //
        W(ServerInfo.ServerId, ServerInfo.ProducerAddress4, ServerInfo.ObserverAddress4);
    }
    void DeserializeMembers() override {  //
        R(ServerInfo.ServerId, ServerInfo.ProducerAddress4, ServerInfo.ObserverAddress4);
    }

    xRelayInfoDispatcherServerInfo ServerInfo;
    //
};

struct xPP_RelayServerHeartBeat : xBinaryMessage {

    void SerializeMembers() override {
        assert((bool)ServerInfo.ServerType && ServerInfo.ServerId && ServerInfo.StartupTimestampMS);
        W(ServerInfo.ServerType);
        W(ServerInfo.ServerId);
        W(ServerInfo.StartupTimestampMS);
        W(ServerInfo.ExportProxyAddress4);
        switch (ServerInfo.ServerType) {
            case eRelayServerType::DEVICE:
                W(ServerInfo.ExportDeviceAddress4);
                W(ServerInfo.ExportDeviceAddress6);
                break;
            default:
                break;
        }
    }

    void DeserializeMembers() override {
        R(ServerInfo.ServerType);
        R(ServerInfo.ServerId);
        R(ServerInfo.StartupTimestampMS);
        R(ServerInfo.ExportProxyAddress4);
        switch (ServerInfo.ServerType) {
            case eRelayServerType::DEVICE:
                R(ServerInfo.ExportDeviceAddress4);
                R(ServerInfo.ExportDeviceAddress6);
                break;
            default:
                break;
        }
    }

    xRelayServerInfoBase ServerInfo = {};
};

struct xPP_RegisterRelayInfoObserver : xBinaryMessage {

    void SerializeMembers() override { W(ChallengeString); }
    void DeserializeMembers() override { R(ChallengeString); }

    std::string ChallengeString;
};

struct xPP_BroadcastRelayInfo : xBinaryMessage {

    void SerializeMembers() override {
        assert((bool)ServerInfo.ServerType && ServerInfo.ServerId && ServerInfo.StartupTimestampMS);
        W(ServerInfo.ServerType);
        W(ServerInfo.ServerId);
        W(ServerInfo.StartupTimestampMS);
        W(ServerInfo.ExportProxyAddress4);
        switch (ServerInfo.ServerType) {
            case eRelayServerType::DEVICE:
                W(ServerInfo.ExportDeviceAddress4);
                W(ServerInfo.ExportDeviceAddress6);
                break;
            default:
                break;
        }
    }

    void DeserializeMembers() override {
        R(ServerInfo.ServerType);
        R(ServerInfo.ServerId);
        R(ServerInfo.StartupTimestampMS);
        R(ServerInfo.ExportProxyAddress4);
        switch (ServerInfo.ServerType) {
            case eRelayServerType::DEVICE:
                R(ServerInfo.ExportDeviceAddress4);
                R(ServerInfo.ExportDeviceAddress6);
                break;
            default:
                break;
        }
    }

    xRelayServerInfoBase ServerInfo = {};
};

struct xPP_BroadcastRelayOffline : xBinaryMessage {
    void SerializeMembers() override { W(ServerId, ServerStartupTimestampMS); }
    void DeserializeMembers() override { R(ServerId, ServerStartupTimestampMS); }

    uint64_t ServerId;
    uint64_t ServerStartupTimestampMS;
};

struct xPP_DownloadDeviceSelectorDispatcherList : xBinaryMessage {
    void SerializeMembers() override { W(Version); }
    void DeserializeMembers() override { R(Version); }

    uint32_t Version;
};

struct xPP_DownloadDeviceSelectorDispatcherListResp : xBinaryMessage {

    void SerializeMembers() override {
        W(Version);

        uint32_t Count = ServerInfoList.size();
        W(Count);

        for (auto & I : ServerInfoList) {
            W(I.ServerId);
            W(I.ExportAddressForClient);
            W(I.ExportAddressForServiceProvider);
        }
    }

    void DeserializeMembers() override {
        R(Version);

        uint32_t Count = 0;
        R(Count);

        if (Count >= MAX_DEVICE_SELECTOR_DISPATCHER_COUNT) {
            GetReader()->SetError();
            return;
        }
        ServerInfoList.resize(Count);
        for (auto & I : ServerInfoList) {
            R(I.ServerId);
            R(I.ExportAddressForClient);
            R(I.ExportAddressForServiceProvider);
        }
    }

    uint32_t                                   Version;
    std::vector<xDeviceSelectorDispatcherInfo> ServerInfoList;
};
