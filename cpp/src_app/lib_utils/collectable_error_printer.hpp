#pragma once

#include <pp_common/_.hpp>

class xCollectableErrorPrinter final : xNonCopyable {

public:
    // xCollectableErrorPrinter(const char * Hint) : ErrorMessage(Hint) {}
    xCollectableErrorPrinter(std::string && Hint) : ErrorMessage(Hint) {}
    xCollectableErrorPrinter(const std::string & Hint) : ErrorMessage(Hint) {}
    ~xCollectableErrorPrinter() = default;

    bool Init(size_t TriggerCountLimit = 1'000, uint64_t TriggerTimeoutLimitMS = 60'000);
    void Clean();
    void Hit();
    void SetLogger(xLogger * Logger) { this->LoggerPtr = Logger; }

private:
    xLogger *         LoggerPtr              = nullptr;
    size_t            Counter                = 0;
    size_t            TriggerCountLimit      = 0;
    uint64_t          LastTriggerTimestampMS = 0;
    uint64_t          TriggerTimeoutLimitMS  = 0;
    const std::string ErrorMessage;
};
