#pragma once
#include "../lib_server_list/audit_target_server_list_downloader.hpp"
#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>

class xAuditTargetClient final {
private:
    struct xRequestContext : xListNode {
        uint64_t RequestId;
        uint64_t StartTimestampMS;
        // source
        uint64_t SourceRequestId;
    };

public:
    bool Init(xIoContext * ICP, const xel::xNetAddress & ServerListAddress);
    void Tick(uint64_t NowMS);
    void Clean();

    bool HasValidBackend() { return CPWConnections; }
    bool Request(uint64_t AuditId, const std::string_view & TargetView, uint64_t Count);

    // callback types
    using xOnEnabled  = std::function<void()>;
    using xOnDisabled = std::function<void()>;
    // callbacks
    xOnEnabled  OnEnabled  = Noop<>;
    xOnDisabled OnDisabled = Noop<>;

private:
    xTicker                          T;
    xAuditTargetServerListDownloader ATSD;
    xClientPoolWrapper               CPW;

    size_t CPWConnections = 0;
};
