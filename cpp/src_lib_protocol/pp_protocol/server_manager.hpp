#pragma once
#include "./base.hpp"

#include <vector>

struct xRegisterAuditServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerUuid, BindAddress);
    }

    void DeserializeMembers() override {
        R(ServerUuid, BindAddress);
    }

    std::string ServerUuid;
    xNetAddress BindAddress;
};

struct xRegisterAuditServerResp : xBinaryMessage {
    void SerializeMembers() override {
        W(Accepted);
    }

    void DeserializeMembers() override {
        R(Accepted);
    }

    bool Accepted = false;
};

struct xRegisterRelayServer : xBinaryMessage {};
struct xRegisterRelayServerResp : xBinaryMessage {
    void SerializeMembers() override {
        W(Accepted);
    }

    void DeserializeMembers() override {
        R(Accepted);
    }

    bool Accepted = false;
};

struct xQueryRelayServerList : xBinaryMessage {};
struct xQueryRelayServerListResp : xBinaryMessage {};

struct xSetMaxAuditId : xBinaryMessage {
    void SerializeMembers() override {
        W(MaxAuditId);
    }

    void DeserializeMembers() override {
        R(MaxAuditId);
    }

    uint32_t MaxAuditId;  // exclusive
};

struct xSetMaxAuditIdResp : xBinaryMessage {
    void SerializeMembers() override {
        W(Accepted);
    }

    void DeserializeMembers() override {
        R(Accepted);
    }

    bool Accepted;  // exclusive
};

struct xQueryAuditServerList : xBinaryMessage {
    uint32_t LastVersion;
    uint32_t LastServerIndex;
};

struct xQueryAuditServerListResp : xBinaryMessage {

    struct xAuditServer {
        uint32_t    BeginAuditId;
        uint32_t    EndAuditId;
        xNetAddress ServerBindAddress;
    };

    void SerializeMembers() override {
        W(Version, TotalServers, FirstServerIndex, LastServerIndex);
    }

    void DeserializeMembers() override {
        R(Version, TotalServers, FirstServerIndex, LastServerIndex);
        for (auto & R : Servers) {
            W(R.BeginAuditId);
            W(R.EndAuditId);
            W(R.ServerBindAddress);
        }
    }

    uint32_t                  Version;
    uint32_t                  TotalServers;
    uint32_t                  FirstServerIndex;
    uint32_t                  LastServerIndex;
    std::vector<xAuditServer> Servers;
};

struct xRegisterErrorReportServer : xBinaryMessage {};
struct xRegisterErrorReportServerResp : xBinaryMessage {
    void SerializeMembers() override {
        W(Accepted);
    }

    void DeserializeMembers() override {
        R(Accepted);
    }

    bool Accepted = false;
};