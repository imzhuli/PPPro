#include "./_global.hpp"
#include "./client_manager.hpp"

#include <pp_protocol/proxy_relay/connection.hpp>

static constexpr const size_t MAX_HTTP_NORMAL_REQUEST_HEADER = 8192;

size_t OnPAC_H_Challenge(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    auto & H   = CC->Http;
    auto   OHS = H.RequestHeader.size();

    H.RequestHeader.append((const char *)DP, DS);
    if (H.RequestHeader.size() <= 4) {
        DEBUG_LOG("too small header");
        return DS;
    }
    auto HeaderEndIndex = H.RequestHeader.find("\r\n\r\n");
    if (HeaderEndIndex == std::string::npos) {
        if (H.RequestHeader.size() >= MAX_HTTP_NORMAL_REQUEST_HEADER) {
            DEBUG_LOG("too big request header");
            return InvalidDataSize;
        }
        DEBUG_LOG("header not finished");
        return DS;
    }

    auto OH       = H.RequestHeader.substr(0, HeaderEndIndex + 4);
    auto Consumed = OH.size() - OHS;
    CC->SuspendReading();

    DEBUG_LOG("OriginalHeaders:\n%s", OH.c_str());

    // rebuild header and put unprocessed body
    auto & RebuiltHeader = H.RequestHeader;
    RebuiltHeader.clear();

    // Line 0:
    auto L0View        = std::string_view(OH.data(), strstr(OH.data(), "\r\n"));
    auto URLStartIndex = L0View.find(' ');
    if (URLStartIndex == L0View.npos) {
        DEBUG_LOG("Invalid Http Target Line (command part)");
        return InvalidDataSize;
    }
    ++URLStartIndex;
    RebuiltHeader.append(L0View.data(), URLStartIndex);

    auto URLStart     = L0View.data() + URLStartIndex;
    auto URLEndIndex  = L0View.find(' ', URLStartIndex);
    auto URLHasSchema = false;
    if (URLEndIndex == L0View.npos) {
        DEBUG_LOG("Invalid Http Target Line (url part)");
        return InvalidDataSize;
    }
    if (0 == strncasecmp(URLStart, "http://", 7)) {
        URLStart      += 7;
        URLStartIndex += 7;
        URLHasSchema   = true;
    }
    auto URLView = std::string_view(URLStart, URLEndIndex - URLStartIndex);
    if (!URLHasSchema) {  // further test unsurported schemas
        auto Schema = URLView.find("://");
        if (Schema != URLView.npos) {
            DEBUG_LOG("unsupported schema");
            return InvalidDataSize;
        }
    }

    auto PathStartIndex = URLView.find('/');
    if (PathStartIndex == URLView.npos) {
        DEBUG_LOG("Invalid Http Target Line (No Path)");
        return InvalidDataSize;
    }

    // get host:port
    for (auto Curr = URLStart; Curr <= URLStart + PathStartIndex; ++Curr) {
        char C = *Curr;
        if (C == ':') {
            CC->Http.TargetHost = { URLStart, (size_t)(Curr - URLStart) };
            CC->Http.TargetPort = atoi(++Curr);
            break;
        } else if (C == '/') {
            CC->Http.TargetHost = { URLStart, (size_t)(Curr - URLStart) };
            CC->Http.TargetPort = 80;
            break;
        }
    }
    RebuiltHeader.append(URLStart + PathStartIndex, L0View.size() - (URLStartIndex + PathStartIndex) + 2);
    DEBUG_LOG("HttpProxy Line0 processing finished: Target: %s:%u", CC->Http.TargetHost.c_str(), (unsigned)CC->Http.TargetPort);

    auto ProxyAuthRequest = std::string();
    auto LineStart        = OH.c_str() + L0View.size() + 2;  // skip \r\n
    while (true) {
        auto LineEnd = strstr(LineStart, "\r\n");
        if (LineEnd == LineStart) {
            DEBUG_LOG("All header processed");
            if (ProxyAuthRequest.empty()) {
                DEBUG_LOG("Invalid Proxy-Authorization: Not Found!");
                CC->PostData(HTTP_407.data(), HTTP_407.size());
                SchedulePassiveKillClientConnection(CC);
                return Consumed;
            }
            if (!PostAuthRequest(CC->ConnectionId, ProxyAuthRequest)) {
                DEBUG_LOG("Failed to query account auth");
                CC->PostData(HTTP_500.data(), HTTP_500.size());
                SchedulePassiveKillClientConnection(CC);
                return Consumed;
            }
            RebuiltHeader.append("\r\n");
            DEBUG_LOG("RebuiltHeader: \n%s", RebuiltHeader.c_str());
            DEBUG_LOG("AuthKey: %s", ProxyAuthRequest.c_str());

            CC->State = CS_H_WAIT_FOR_AUTH_RESULT;
            return Consumed;
        }

        auto LineRef = std::string_view(LineStart, LineEnd);
        if (LineRef.size() > 18 && 0 == strncasecmp(LineRef.data(), "Proxy-Connection: ", 18)) {
            Pass();
        } else if (LineRef.size() > 12 && 0 == strncasecmp(LineRef.data(), "Connection: ", 12)) {
            RebuiltHeader.append("Connection: close\r\n", 19);
        } else if (LineRef.size() > 21 && 0 == strncasecmp(LineRef.data(), "Proxy-Authorization: ", 21)) {
            auto AuthStart  = LineRef.data() + 21;
            auto AuthLength = LineRef.size() - 21;
            if (AuthLength < 6 && 0 != strncasecmp(AuthStart, "Basic ", 6)) {
                DEBUG_LOG("Invalid Proxy-Authorization Request");
                return InvalidDataSize;
            }
            auto Base64Start = AuthStart + 6;
            auto Base64Size  = AuthLength - 6;
            ProxyAuthRequest = xel::Base64Decode(Base64Start, Base64Size);
            if (ProxyAuthRequest.empty()) {
                DEBUG_LOG("Invalid Proxy-Authorization Request");
                return InvalidDataSize;
            }
            auto NameEndIndex = ProxyAuthRequest.find(':');
            if (NameEndIndex == std::string::npos) {
                DEBUG_LOG("HttpReqeust AuthInfo Not Found!");
                return InvalidDataSize;
            }
        } else {
            RebuiltHeader.append(LineRef);
            RebuiltHeader.append("\r\n", 2);
        }

        LineStart = LineEnd + 2;
    }
    Unreachable();
    return InvalidDataSize;
}

void OnPAC_H_AuthResult(xPA_ClientConnection * CC, const xClientAuthResult * AR) {
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
        CC->PostData(HTTP_500.data(), HTTP_500.size());
        SchedulePassiveKillClientConnection(CC);
        return;
    }
    CC->State = CS_H_WAIT_FOR_DEVICE_RESULT;
}

void OnPAC_H_DeviceResult(xPA_ClientConnection * CC, const xDeviceSelectorResult & Result) {
    if (!Result) {
        CC->PostData(HTTP_500.data(), HTTP_500.size());
        SchedulePassiveKillClientConnection(CC);
        return;
    }
    CC->DeviceRelayServerRuntimeId = Result.DeviceRelayServerRuntimeId;
    CC->DeviceRelaySideId          = Result.DeviceRelaySideId;

    RequestRelayTargetConnection(CC->ConnectionId, CC->DeviceRelayServerRuntimeId, CC->DeviceRelaySideId, CC->Http.TargetHost, CC->Http.TargetPort);
    CC->State = CS_H_WAIT_FOR_CONECTION_ESTABLISH;
}

void OnPAC_H_ConnectionResult(xPA_ClientConnection * CC, uint64_t RelaySideContextId) {
    if (!RelaySideContextId) {
        DEBUG_LOG("Connection refused");
        CC->PostData(HTTP_502.data(), HTTP_502.size());
        SchedulePassiveKillClientConnection(CC);
        return;
    }

    DEBUG_LOG("Connection established");
    CC->RelaySideContextId = RelaySideContextId;
    CC->State              = CS_T_READY;

    auto & H = CC->Http.RequestHeader;
    if (!RequestRelayPostConnectionData(CC->ConnectionId, CC->DeviceRelayServerRuntimeId, CC->RelaySideContextId, H.data(), H.size())) {
        DEBUG_LOG("failed to post rebuilt header");
        SchedulePassiveKillClientConnection(CC);
        return;
    }
    H.clear();

    CC->ResumeReading();
    KeepAlive(CC);
}

size_t OnPAC_H_UploadData(xPA_ClientConnection * CC, ubyte * DP, size_t DS) {
    if (!RequestRelayPostConnectionData(CC->ConnectionId, CC->DeviceRelayServerRuntimeId, CC->RelaySideContextId, DP, DS)) {
        DEBUG_LOG("failed to post PushData message to relay");
        return InvalidDataSize;
    }
    KeepAlive(CC);
    return DS;
}
