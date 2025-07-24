#pragma once
#include "../base.hpp"

struct xCC_PA_RelayServerInfo {
    uint32_t      RuntimeServerId;
    xSourcePoolId SourcePoolId;
    xNetAddress   ExportProxyAddress;
};

struct xCC_PA_GetRelayServerListVersion : xBinaryMessage {
    void SerializeMembers() override {
        W(OldVersion);
    };
    void DeserializeMembers() override {
        R(OldVersion);
    };
    xVersion OldVersion = 0;
};

struct xCC_PA_GetRelayServerListVersionResp : xBinaryMessage {
    void SerializeMembers() override {
        W(Version);
    };
    void DeserializeMembers() override {
        R(Version);
    };
    xVersion Version = 0;
};

struct xCC_PA_DownloadRelayServerList : xBinaryMessage {

    void SerializeMembers() override {
        W(Version);
        W(StartIndex);
    };

    void DeserializeMembers() override {
        R(Version);
        R(StartIndex);
    };

    xVersion Version = 0;
    uint32_t StartIndex;

    //
};

struct xCC_PA_DownloadRelayServerListRespWriter : xBinaryMessage {

    static const size_t MAX_SERVER_INFO_COUNT_PER_DOWNLOAD = 100;

    void SerializeMembers() override {
        assert(TotalServerList);
        uint32_t Total = (uint32_t)TotalServerList->size();
        assert(StartIndex < Total);

        uint32_t EndIndex = StartIndex + MAX_SERVER_INFO_COUNT_PER_DOWNLOAD;
        EndIndex          = std::min(EndIndex, Total);
        uint32_t Count    = EndIndex - StartIndex;

        W(Version);
        W(Total);
        W(StartIndex);
        W(Count);
        for (size_t I = StartIndex; I < EndIndex; ++I) {
            auto & Info = (*TotalServerList)[I];
            W(Info.RuntimeServerId);
            W(Info.SourcePoolId);
            W(Info.ExportProxyAddress);
        }
    };

    xVersion                                    Version = 0;
    uint32_t                                    StartIndex;
    const std::vector<xCC_PA_RelayServerInfo> * TotalServerList;

    //
};

struct xCC_PA_DownloadRelayServerListRespReader : xBinaryMessage {

    void DeserializeMembers() override {
        R(Version);
        R(Total);
        R(StartIndex);
        R(Count);
        RenewValue(LocalServerList, Count);
        for (uint32_t I = 0; I < Count; ++I) {
            auto & Info = LocalServerList[I];
            R(Info.RuntimeServerId);
            R(Info.SourcePoolId);
            R(Info.ExportProxyAddress);
        }
    };

    uint32_t GetNextStartIndex() const {
        return StartIndex + Count;
    }

    xVersion                            Version = 0;
    uint32_t                            Total   = 0;
    uint32_t                            StartIndex;
    uint32_t                            Count;
    std::vector<xCC_PA_RelayServerInfo> LocalServerList;

    //
};

std::string ToString(const std::vector<xCC_PA_RelayServerInfo> & List);
