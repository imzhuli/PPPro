#pragma once
#include "../lib_utils/all.hpp"

using xServerIdPoster = std::function<void(xMessagePoster * Poster, uint64_t LocalServerId)>;

class xRegisterServerClient
    : public xClient
    , public xMessagePoster {
public:
    using xClient::Init;
    void Clean();

    void SetServerIdPoster(const xServerIdPoster & Poster) { IdPoster = Poster; }
    void SetLocalServerId(uint64_t NewServerId);

private:
    void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) override;
    void OnServerConnected() override;

private:
    uint64_t        LocalServerId = 0;
    xServerIdPoster IdPoster;
};
