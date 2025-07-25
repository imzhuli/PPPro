#pragma once
#include "../base.hpp"

struct xAuditAccountUsage : xBinaryMessage {

    void SerializeMembers() override {
        W(AuditId, UploadSize, DownloadSize, TcpIncreament, UdpIncreament, DeviceChangeIncreament);  //
    }
    void DeserializeMembers() override {
        R(AuditId, UploadSize, DownloadSize, TcpIncreament, UdpIncreament, DeviceChangeIncreament);  //
    }

    xAuditId AuditId                = {};
    uint64_t UploadSize             = {};
    uint64_t DownloadSize           = {};
    uint64_t TcpIncreament          = {};
    uint64_t UdpIncreament          = {};
    uint64_t DeviceChangeIncreament = {};
    //
};

struct xAuditAccountTarget : xBinaryMessage {

    void SerializeMembers() override {
        W(AuditId, Target, LocalCount);  //
    }
    void DeserializeMembers() override {
        R(AuditId, Target, LocalCount);  //
    }

    xAuditId         AuditId    = {};
    std::string_view Target     = {};
    uint64_t         LocalCount = {};

    //
};
