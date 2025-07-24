#include "./auth_service.hpp"

#include <pp_protocol/_backend/auth_by_user_pass.hpp>
#include <pp_protocol/command.hpp>
#include <pp_protocol/internal/all.hpp>

static constexpr const uint64_t LOCAL_AUDIT_TIMEOUT_MS = 60'000;

bool xAC_AuthService::Init(xIoContext * ICP, const xNetAddress & BindAddress) {
    RuntimeAssert(xService::Init(ICP, BindAddress, 10000, true));
    RuntimeAssert(CacheManager.Init(100'000, 100'000));
    RuntimeAssert(BackendPool.Init(ICP, MAX_BACKEND_SERVER_COUNT));

    CacheManager.SetAsyncQueryCallback([this](uint64_t CacheNodeId, const std::string & Key) {
        auto T     = xPPB_BackendAuthByUserPass();
        T.UserPass = Key;
        BackendPool.PostMessage(Cmd_BackendAuthByUserPass, CacheNodeId, T);
        return true;
    });

    CacheManager.SetOnImmediateResultCallback([this](const xCacheRequestContext & Context, const void * Data) {
        DEBUG_LOG("ImmediateResultCallback");
        auto   RD   = (xPPB_BackendAuthByUserPassResp *)Data;
        auto   Resp = xPP_QueryAuthCacheResp();
        auto & R    = Resp.Result;

        R.AuditId      = RD->AuditId;
        R.CountryId    = RD->CountryId;
        R.StateId      = RD->StateId;
        R.CityId       = RD->CityId;
        R.RequireIpv6  = RD->Ipv6Prefered;
        R.RequireUdp   = RD->EnableUdp;
        R.AutoChangeIp = RD->AutoChangeIpOnDeviceOffline;
        R.PAToken      = RD->ServerToken;

        PostMessage(Context.Value.U64, Cmd_AuthService_QueryAuthCacheResp, Context.ValueEx.U64, Resp);
        return;
    });
    CacheManager.SetOnAsyncResultCallback([this](const xCacheRequestContext & Context, const void * Data) {
        DEBUG_LOG("AsyncResultCallback");
        auto   RD   = (xPPB_BackendAuthByUserPassResp *)Data;
        auto   Resp = xPP_QueryAuthCacheResp();
        auto & R    = Resp.Result;

        R.AuditId      = RD->AuditId;
        R.CountryId    = RD->CountryId;
        R.StateId      = RD->StateId;
        R.CityId       = RD->CityId;
        R.RequireIpv6  = RD->Ipv6Prefered;
        R.RequireUdp   = RD->EnableUdp;
        R.AutoChangeIp = RD->AutoChangeIpOnDeviceOffline;
        R.PAToken      = RD->ServerToken;

        PostMessage(Context.Value.U64, Cmd_AuthService_QueryAuthCacheResp, Context.ValueEx.U64, Resp);
        return;
    });
    CacheManager.SetOnReleaseDataCallback([this](uint64_t CacheNodeId, const void * Data) {
        DEBUG_LOG("ReleaseDataCallback");
        delete (xPPB_BackendAuthByUserPassResp *)Data;
        ++DeleteCachedResultCount;
        return;
    });

    BackendPool.SetBackendPacketCallback([this](xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
        OnBackendPacket(CommandId, RequestId, PayloadPtr, PayloadSize);
    });

    return true;
}

void xAC_AuthService::Clean() {
    BackendPool.Clean();
    CacheManager.Clean();

    Reset(LastOutputLocalAuditTimestampMS);
    Reset(NewCachedResultCount);
    Reset(DeleteCachedResultCount);
}

void xAC_AuthService::OnTick(uint64_t NowMS) {
    CacheManager.RemoveTimeoutCacheNodes(NowMS);
    TickAll(NowMS, BackendPool);

    // do local audit:
    if (NowMS - LastOutputLocalAuditTimestampMS >= LOCAL_AUDIT_TIMEOUT_MS) {
        auto LocalCacheAudit = CacheManager.GetLocalAudit();
        AuditLogger->I("New/Del Result=%zi:%zi, %s", NewCachedResultCount, DeleteCachedResultCount, LocalCacheAudit.ToString().c_str());
        LastOutputLocalAuditTimestampMS = NowMS;
    }
}

bool xAC_AuthService::OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    DEBUG_LOG("%" PRIx64 ":%" PRIx64 ", \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());

    if (CommandId != Cmd_AuthService_QueryAuthCache) {
        DEBUG_LOG("Invalid command id");
        return true;
    }

    auto Req = xPP_QueryAuthCache();
    if (!Req.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("Invalid request");
        return true;
    }

    CacheManager.PostAcquireCacheNodeRequest(std::string(Req.UserPass), { { .U64 = Connection.GetConnectionId() }, { .U64 = RequestId } });
    return true;
}

void xAC_AuthService::PostResposne(xServiceClientConnection & Connection, xPacketRequestId RequestId, const xClientAuthResult * NP) {
    auto Resp   = xPP_QueryAuthCacheResp();
    Resp.Result = *NP;
    PostMessage(Connection, Cmd_AuthService_QueryAuthCacheResp, RequestId, Resp);
}

void xAC_AuthService::OnBackendPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
    switch (CommandId) {
        case Cmd_BackendAuthByUserPassResp:
            return OnCmdAuthByUserPassResp(CommandId, RequestId, PayloadPtr, PayloadSize);

        default:
            X_DEBUG_PRINTF("unsupported protocol command");
            break;
    }
    return;
}

void xAC_AuthService::OnCmdAuthByUserPassResp(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {

    DEBUG_LOG("%s", HexShow(PayloadPtr, PayloadSize).c_str());

    auto P = xPPB_BackendAuthByUserPassResp();
    if (!P.Deserialize(PayloadPtr, PayloadSize)) {
        DEBUG_LOG("invalid protocol");
        return;
    }
    auto PC = new xPPB_BackendAuthByUserPassResp(P);
    DEBUG_LOG("%s", PC->ToString().c_str());

    ++NewCachedResultCount;
    CacheManager.SetAsyncResultData(RequestId, PC);
}

void xAC_AuthService::UpdateBackendAuthInfo(const std::string & AppKey, const std::string & AppSecret) {
    this->AppKey    = AppKey;
    this->AppSecret = AppSecret;
}

void xAC_AuthService::UpdateBackendServerList(const std::vector<xNetAddress> & Added, const std::vector<xNetAddress> & Removed) {
    for (auto & R : Removed) {
        BackendPool.RemoveServer(R);
    }
    for (auto & A : Added) {
        BackendPool.AddServer(A, AppKey, AppSecret);
    }
}
