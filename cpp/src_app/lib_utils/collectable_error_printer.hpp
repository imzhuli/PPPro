#pragma once

#include <pp_common/_.hpp>

class xCollectableErrorPrinter final : xNonCopyable {

public:
    // xCollectableErrorPrinter(const char * Hint) : ErrorMessage(Hint) {}
    xCollectableErrorPrinter(std::string && Hint) : ErrorMessage(Hint) {}
    xCollectableErrorPrinter(const std::string & Hint) : ErrorMessage(Hint) {}
    ~xCollectableErrorPrinter() = default;

    bool Init(size_t TriggerCountLimit = 1'000, uint64_t TriggerTimeoutLimitMS = 60'000);
    bool Init(xLogger * LoggerPtr, size_t TriggerCountLimit, uint64_t TriggerTimeoutLimitMS);
    void Clean();
    void Hit();

private:
    xLogger *         LoggerPtr              = nullptr;
    size_t            Counter                = 0;
    size_t            TriggerCountLimit      = 0;
    uint64_t          LastTriggerTimestampMS = 0;
    uint64_t          TriggerTimeoutLimitMS  = 0;
    const std::string ErrorMessage;
};
