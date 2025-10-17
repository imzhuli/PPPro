#include "./client_udp_channel_manager.hpp"

#include "./_global.hpp"
#include "./client_manager.hpp"

struct xPA_UdpChannel
    : xUdpChannel
    , xPA_UdpChannelBinding {
    xPA_ClientConnection * Owner = nullptr;
};

static std::vector<std::pair<xNetAddress, xNetAddress>> UdpMappingList;
static size_t                                           UdpMappingIndex = 0;

static struct xPA_UdpChannelListener : xUdpChannel::iListener {
    void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override {}
} UdpChannelListener;

static bool InitUdpChannelManager(const std::vector<std::pair<xNetAddress, xNetAddress>> & AddressMapping) {
    UdpMappingList = AddressMapping;
    for (auto & U : UdpMappingList) {
        RuntimeAssert(!U.first.Port && !U.second.Port);
    }
    return true;
}

bool InitUdpChannelManager() {
    std::vector<std::pair<xNetAddress, xNetAddress>> AddressMapping;

    auto Segs = Split(ConfigUdpMapping, ",");
    for (auto MS : Segs) {
        auto S2 = Split(MS, "-");
        if (S2.size() != 2) {
            DEBUG_LOG("invalid seg");
            continue;
        }

        auto S = xNetAddress::Parse(Trim(S2[0]));
        auto D = xNetAddress::Parse(Trim(S2[1]));

        DEBUG_LOG("%s -> %s", S.IpToString().c_str(), D.IpToString().c_str());
        AddressMapping.push_back(std::make_pair(S, D));
    }

    return InitUdpChannelManager(AddressMapping);
}

void CleanUdpChanneManager() {
    Reset(UdpMappingList);
}

xPA_UdpChannelBinding * CreateUdpChannel(void * Owner) {
    if (UdpMappingList.empty()) {
        return nullptr;
    }

    auto UC = new (std::nothrow) xPA_UdpChannel();
    if (!UC) {
        return nullptr;
    }
    auto & M = UdpMappingList[UdpMappingIndex++];
    if (UdpMappingIndex == UdpMappingList.size()) {
        UdpMappingIndex = 0;
    }

    if (!UC->Init(ServiceIoContext, M.first, &UdpChannelListener)) {
        delete UC;
        return nullptr;
    }
    UC->ExportBindingAddress      = M.second;
    UC->ExportBindingAddress.Port = UC->GetBindAddress().Port;
    DEBUG_LOG("ExportBindAddress: %s", UC->ExportBindingAddress.ToString().c_str());

    UC->Owner = static_cast<xPA_ClientConnection *>(Owner);
    return UC;
}

void DestroyUdpChannel(xPA_UdpChannelBinding * UB) {
    auto UC = static_cast<xPA_UdpChannel *>(UB);
    UC->Clean();
    delete UC;
}
