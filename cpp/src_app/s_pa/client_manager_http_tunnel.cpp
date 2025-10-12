#include "./_global.hpp"
#include "./client_manager.hpp"

#include <pp_protocol/proxy_relay/connection.hpp>

static constexpr const size_t MAX_HTTP_TUNNEL_REQUEST_HEADER = 1024;
static constexpr auto         HTTP_407 = "HTTP/1.1 407 Proxy Authentication Required\r\nProxy-Authenticate: Basic realm=Restricted\r\nConnection: close\r\n\r\n"sv;
static constexpr auto         HTTP_502 = "HTTP/1.1 502 Target Unreached"sv;
static constexpr auto         HTTP_200 = "HTTP/1.1 200 Connection established\r\nProxy-agent: proxy / 1.0\r\n\r\n"sv;

size_t OnPAC_T_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    auto & H = CC->Http;
    H.RequestHeader.append((const char *)DP, DS);
    if (H.RequestHeader.size() >= MAX_HTTP_TUNNEL_REQUEST_HEADER) {
        DEBUG_LOG("request header oversized");
        return InvalidDataSize;
    }
    if (H.RequestHeader.size() <= 4) {
        DEBUG_LOG("too small header");
        return DS;
    }

    DEBUG_LOG("Header: \n%s", H.RequestHeader.c_str());
    auto HeaderEndStart = H.RequestHeader.size() - 4;
    if (strcmp(H.RequestHeader.c_str() + HeaderEndStart, "\r\n\r\n")) {
        DEBUG_LOG("header not finished");
        return DS;
    }

    // process first line:
    auto L0 = H.RequestHeader.c_str();
    if (strncasecmp("CONNECT ", L0, 8)) {
        DEBUG_LOG("invalid connect request line");
        return InvalidDataSize;
    }
    L0 += 8;  // skip "CONNECT "

    auto L0End        = strstr(L0, "\r\n");
    auto HostnameView = std::string_view();
    auto Port         = uint16_t(0);
    for (auto Curr = L0; Curr < L0End; ++Curr) {
        char C = *Curr;
        if (C == ':') {
            HostnameView = { L0, (size_t)(Curr - L0) };
            Port         = atoi(++Curr);
            break;
        } else if (C == ' ') {
            HostnameView = { L0, (size_t)(Curr - L0) };
            Port         = 80;
            break;
        }
    }
    if (!HostnameView.length()) {
        DEBUG_LOG("Invalid HttpProxy Target");
        return InvalidDataSize;
    }
    H.TargetHost = std::string(HostnameView);
    H.TargetPort = Port;
    DEBUG_LOG("CONNECT target: %s:%u", H.TargetHost.c_str(), (unsigned)H.TargetPort);

    // process from line 1:
    auto ProxyAuthRequest = std::string();
    auto LineStart        = L0End + 2;
    while (true) {
        auto LineEnd = strstr(LineStart, "\r\n");
        if (LineEnd == LineStart) {  // header reading finished
            if (ProxyAuthRequest.empty()) {
                DEBUG_LOG("Invalid Proxy-Authorization: Not Found!");
                CC->PostData(HTTP_407.data(), HTTP_407.size());
                SchedulePassiveKillClientConnection(CC);
                return 0;
            }

            if (!PostAuthRequest(CC->ConnectionId, ProxyAuthRequest)) {
                DEBUG_LOG("Failed to query account auth");
                DeferKillClientConnection(CC);
                return 0;
            }
            DEBUG_LOG("AuthKey: %s", ProxyAuthRequest.c_str());
            CC->State = CS_T_WAIT_FOR_AUTH_RESULT;
            return DS;
        }

        auto LineLength = LineEnd - LineStart;
        if (LineLength > 21 && 0 == strncasecmp(LineStart, "Proxy-Authorization: ", 21)) {
            auto AuthStart  = LineStart + 21;
            auto AuthLength = LineLength - 21;
            if (AuthLength < 6 && 0 != strncasecmp(AuthStart, "Basic ", 6)) {
                DEBUG_LOG("Invalid Proxy-Authorization Request");
                DeferKillClientConnection(CC);
                return 0;
            }
            auto Base64Start = AuthStart + 6;
            auto Base64Size  = AuthLength - 6;
            ProxyAuthRequest = Base64Decode(Base64Start, Base64Size);
            if (ProxyAuthRequest.empty()) {
                DEBUG_LOG("Invalid Proxy-Authorization Request");
                DeferKillClientConnection(CC);
                return 0;
            }
            auto NameEndIndex = ProxyAuthRequest.find(':');
            if (NameEndIndex == std::string::npos) {
                DEBUG_LOG("HttpReqeust AuthInfo Not Found!");
                DeferKillClientConnection(CC);
                return 0;
            }
        }

        LineStart = LineEnd + 2;
    }
    Unreachable();
    return 0;
}

void OnPAC_T_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR) {
    if (!AR) {
        CC->PostData(HTTP_407.data(), HTTP_407.size());
        SchedulePassiveKillClientConnection(CC);
        return;
    }

    // select device:
    auto OPS      = xDeviceSelectorOptions();
    OPS.CountryId = AR->CountryId;
    OPS.StateId   = AR->StateId;
    OPS.CityId    = AR->CityId;
    if (AR->RequireIpv6) {
        OPS.StrategyFlags |= DSS_IPV6;
    } else {
        OPS.StrategyFlags |= DSS_IPV4;
    }
    if (AR->RequireUdp) {
        OPS.StrategyFlags |= DSS_UDP;
    }
    if (AR->PersistentDeviceBinding) {
        OPS.StrategyFlags |= DSS_DEVICE_PERSISTENT;
    } else {
        if (!AR->AlwaysChangeIp) {
            OPS.StrategyFlags |= DSS_DEVICE_VOLATILE;
        }
    }
    if (!PostDeviceSelectorRequest(CC->ConnectionId, OPS)) {
        CC->PostData(HTTP_407.data(), HTTP_407.size());
        SchedulePassiveKillClientConnection(CC);
        return;
    }
    CC->State = CS_T_WAIT_FOR_DEVICE_RESULT;
}

void OnPAC_T_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result) {
    if (!Result) {
        CC->PostData(HTTP_407.data(), HTTP_407.size());
        SchedulePassiveKillClientConnection(CC);
        return;
    }
    CC->DeviceRelayServerRuntimeId = Result.DeviceRelayServerRuntimeId;
    CC->DeviceRelaySideId          = Result.DeviceRelaySideId;

    RequestRelayTargetConnection(CC->ConnectionId, CC->DeviceRelayServerRuntimeId, CC->DeviceRelaySideId, CC->Http.TargetHost, CC->Http.TargetPort);
    CC->State = CS_T_WAIT_FOR_CONECTION_ESTABLISH;
}

void OnPAC_T_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId) {
    if (!RelaySideContextId) {
        DEBUG_LOG("Connection refused");
        CC->PostData(HTTP_502.data(), HTTP_502.size());
        SchedulePassiveKillClientConnection(CC);
        return;
    }

    DEBUG_LOG("Connection established");
    CC->PostData(HTTP_200.data(), HTTP_200.size());
    CC->RelaySideContextId = RelaySideContextId;
    CC->State              = CS_T_READY;
    KeepAlive(CC);
}

size_t OnPAC_T_UploadData(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    auto Consumed = size_t();
    while (DS) {
        auto MaxPushSize = std::min(DS, xPR_PushData::MAX_PAYLOAD_SIZE);
        if (!MaxPushSize) {
            break;
        }
        auto P               = xPR_PushData();
        P.ProxySideContextId = CC->ConnectionId;
        P.RelaySideContextId = CC->RelaySideContextId;
        P.PayloadView        = std::string_view((const char *)DP, MaxPushSize);
        if (!PostRelayMessage(CC->DeviceRelayServerRuntimeId, Cmd_PA_RL_PostData, 0, P)) {
            DEBUG_LOG("failed to post PushData message to relay");
            return InvalidDataSize;
        }

        DP       += MaxPushSize;
        DS       -= MaxPushSize;
        Consumed += MaxPushSize;
    }
    KeepAlive(CC);
    return Consumed;
}
