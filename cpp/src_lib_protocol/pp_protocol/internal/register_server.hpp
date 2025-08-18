#pragma once
#include "../base.hpp"

struct xPP_RegisterAuthCacheServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
        W(Address);
    }

    void DeserializeMembers() override {
        R(ServerId);
        R(Address);
    }

    uint64_t    ServerId;
    xNetAddress Address;
    //
};

struct xPP_RegisterAuditDeviceServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
        W(Address);
    }

    void DeserializeMembers() override {
        R(ServerId);
        R(Address);
    }

    uint64_t    ServerId;
    xNetAddress Address;

    //
};

struct xPP_RegisterAuditAccountServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
        W(Address);
    }

    void DeserializeMembers() override {
        R(ServerId);
        R(Address);
    }

    uint64_t    ServerId;
    xNetAddress Address;

    //
};

struct xPP_RegisterDeviceStateRelayServer : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerId);
        W(Address);
        W(ObserverAddress);
    }

    void DeserializeMembers() override {
        R(ServerId);
        R(Address);
        R(ObserverAddress);
    }

    uint64_t    ServerId        = {};
    xNetAddress Address         = {};
    xNetAddress ObserverAddress = {};

    //
};

struct xPP_RegisterRelayInfoDispatcherServer : xBinaryMessage {
    void SerializeMembers() override {  //
        W(ServerInfo.ServerId, ServerInfo.ProducerAddress4, ServerInfo.ObserverAddress4);
        W(ServerInfo.ServerId, ServerInfo.ProducerAddress6, ServerInfo.ObserverAddress6);
    }

    void DeserializeMembers() override {  //
        R(ServerInfo.ServerId, ServerInfo.ProducerAddress4, ServerInfo.ObserverAddress4);
        R(ServerInfo.ServerId, ServerInfo.ProducerAddress6, ServerInfo.ObserverAddress6);
    }
    xRelayInfoDispatcherServerInfo ServerInfo;
    //
};

struct xPP_RegisterDeviceSelectorDispatcher : xBinaryMessage {

    void SerializeMembers() override {
        W(ServerInfo.ServerId);
        W(ServerInfo.ExportAddressForClient);
        W(ServerInfo.ExportAddressForServiceProvider);
    }
    void DeserializeMembers() override {
        R(ServerInfo.ServerId);
        R(ServerInfo.ExportAddressForClient);
        R(ServerInfo.ExportAddressForServiceProvider);
    }

    xDeviceSelectorDispatcherInfo ServerInfo;
    //
};

////////////////////

struct xPP_RegisterServerResp : xBinaryMessage {

    void SerializeMembers() override { W(Accepted); }
    void DeserializeMembers() override { R(Accepted); }

    bool Accepted;
};
