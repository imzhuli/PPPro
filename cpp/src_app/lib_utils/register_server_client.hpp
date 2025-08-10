#pragma once
#include "../lib_utils/all.hpp"

using xServerIdPoster = std::function<void(const xMessagePoster & Poster, uint64_t LocalServerId)>;

class xRegisterServerClient;
class xRegisterServerClientMessagePoster;

class xRegisterServerClientMessagePoster final : protected xMessagePoster {

public:
    xRegisterServerClientMessagePoster(xRegisterServerClient * RSC) : Owner(RSC) {}

private:
    friend class xRegisterServerClient;
    void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const override;

    xRegisterServerClient * Owner = nullptr;
};

class xRegisterServerClient final : public xClient {
public:
    using xClient::Init;
    void Clean();

    void SetServerIdPoster(const xServerIdPoster & Poster) { IdPoster = Poster; }
    void SetLocalServerId(uint64_t NewServerId);

private:
    void OnServerConnected() override;

private:
    uint64_t        LocalServerId = 0;
    xServerIdPoster IdPoster;
};
