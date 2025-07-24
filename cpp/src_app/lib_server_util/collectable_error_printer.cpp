#include "./collectable_error_printer.hpp"

static auto DefaultLogger          = xBaseLogger();
static auto DefaultLoggerCounter   = size_t(0);
static auto DefaultLoggerInitMutex = std::mutex();

static void InitDefaultLogger() {
    auto LG = std::lock_guard(DefaultLoggerInitMutex);
    if (0 == DefaultLoggerCounter++) {
        DefaultLogger.Init();
        DefaultLogger.SetLogLevel(eLogLevel::Warning);
    }
}

static void CleanDefaultLogger() {
    auto LG = std::lock_guard(DefaultLoggerInitMutex);
    if (!--DefaultLoggerCounter) {
        DefaultLogger.Clean();
    }
}

bool xCollectableErrorPrinter::Init(size_t TriggerCountLimit, uint64_t TriggerTimeoutLimitMS) {
    return Init(&DefaultLogger, TriggerCountLimit, TriggerTimeoutLimitMS);
}

bool xCollectableErrorPrinter::Init(xLogger * LoggerPtr, size_t TriggerCountLimit, uint64_t TriggerTimeoutLimitMS) {
    InitDefaultLogger();

    this->LoggerPtr             = LoggerPtr;
    this->TriggerCountLimit     = TriggerCountLimit;
    this->TriggerTimeoutLimitMS = TriggerTimeoutLimitMS;

    return true;
}

void xCollectableErrorPrinter::Clean() {
    Reset(LoggerPtr);
    Reset(Counter);
    Reset(TriggerCountLimit);
    Reset(LastTriggerTimestampMS);
    Reset(TriggerTimeoutLimitMS);

    CleanDefaultLogger();
}

void xCollectableErrorPrinter::Hit() {
    ++Counter;
    auto Output = false;
    auto NowMS  = GetTimestampMS();

    if (Counter > TriggerCountLimit) {
        Output = true;
    }

    if (TriggerTimeoutLimitMS && NowMS - LastTriggerTimestampMS > TriggerTimeoutLimitMS) {
        Output = true;
    }

    if (Output) {
        X_PERROR(" (%zi) %s", Counter, ErrorMessage.c_str());
        LastTriggerTimestampMS = NowMS;
        Counter                = 0;
    }
}
