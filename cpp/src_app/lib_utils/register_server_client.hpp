#pragma once
#include "../lib_utils/all.hpp"

class xRegisterServerClient final : public xClient {
public:
    using xClient::Init;
    void Clean();
    void SetLocalServerId(uint64_t NewServerId);

    using xServerRegister = std::function<void(const xMessageChannel & Poster, uint64_t LocalServerId)>;

    xServerRegister ServerRegister = Noop<>;

private:
    void OnServerConnected() override;

private:
    uint64_t LocalServerId = 0;
};
