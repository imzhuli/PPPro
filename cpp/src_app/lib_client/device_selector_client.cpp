#include "./device_selector_client.hpp"

#include "./auth_client.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>

static constexpr const uint64_t RequestTimeoutMS = 1'000;
static constexpr const size_t   MaxRequestCount  = 10'0000;

bool xDeviceSelectorClient::Init(xIoContext * ICP, const xel::xNetAddress & ServerListAddress) {
    RuntimeAssert(RequestPool.Init(MaxRequestCount));
    if (!DSD.Init(ICP, ServerListAddress)) {
        return false;
    }
    if (!ACC.Init(ICP)) {
        DSD.Clean();
        return false;
    }

    DSD.OnUpdateDeviceSelectorDispatcherServerListCallback = [this](uint32_t Version, const std::vector<xDeviceSelectorDispatcherInfo> & ServerList) {
        std::vector<xNetAddress> AL;
        for (auto S : ServerList) {
            AL.push_back(S.ExportAddressForClient);
        }
        std::sort(AL.begin(), AL.end());
        ACC.UpdateServerList(AL);
    };

    ACC.OnConnectedCallback = [this](auto &) {
        if (!ACCConnections++) {
            OnEnabled();
        }
    };
    ACC.OnDisonnectedCallback = [this](auto &) {
        if (!--ACCConnections) {
            OnDisabled();
        }
    };
    ACC.OnPacketCallback = [this](const xMessageChannel & Source, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        DEBUG_LOG("CommondId=%" PRIx32 ", RequestId:%" PRIx64 " Data=\n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
        if (CommandId != Cmd_AuthService_QueryAuthCacheResp) {
            DEBUG_LOG("Invalid server response command");
            return false;
        }
        auto Resp = xPP_QueryAuthCacheResp();
        if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
            Logger->E("Invalid server response protocol");
            return false;
        }
        auto P = RequestPool.CheckAndGet(RequestId);
        if (!P) {
            DEBUG_LOG("invalid request id");
            return true;
        }
        OnAuthCacheResultCallback(P->SourceRequestId, &Resp.Result);
        RequestPool.Release(P->RequestId);
        return true;
    };

    return true;
}

void xDeviceSelectorClient::Tick(uint64_t NowMS) {
    T.Update(NowMS);
    DSD.Tick(NowMS);
    ACC.Tick(NowMS);

    auto KillCond = [KT = NowMS - RequestTimeoutMS](const xRequestContext & C) { return C.StartTimestampMS <= KT; };
    while (auto P = static_cast<xRequestContext *>(RequestQueue.PopHead(KillCond))) {
        DEBUG_LOG("Remove request: %" PRIx64 "", P->SourceRequestId);
        OnAuthCacheResultCallback(P->SourceRequestId, nullptr);
        RequestPool.Release(P->RequestId);
    }
}

void xDeviceSelectorClient::Clean() {
    Reset(ACCConnections);
    ACC.Clean();
    DSD.Clean();
    RequestPool.Clean();
}

bool xDeviceSelectorClient::Request(const std::string_view & UserPassword, uint64_t SourceRequestId) {
    if (!ACCConnections) {
        return false;
    }
    auto RID = RequestPool.Acquire();
    if (!RID) {
        return false;
    }
    auto & R           = RequestPool[RID];
    R.RequestId        = RID;
    R.StartTimestampMS = T();
    R.SourceRequestId  = SourceRequestId;
    RequestQueue.AddTail(R);

    auto T     = xPP_QueryAuthCache();
    T.UserPass = UserPassword;
    auto H     = HashString(UserPassword);

    ACC.PostMessageByHash(H, Cmd_AuthService_QueryAuthCache, RID, T);
    return true;
}
