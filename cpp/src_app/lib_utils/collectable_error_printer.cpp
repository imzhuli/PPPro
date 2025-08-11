#include "./collectable_error_printer.hpp"

bool xCollectableErrorPrinter::Init(size_t TriggerCountLimit, uint64_t TriggerTimeoutLimitMS) {
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

    if (Output && LoggerPtr) {
        LoggerPtr->E(" (%zi) %s", Counter, ErrorMessage.c_str());
        LastTriggerTimestampMS = NowMS;
        Counter                = 0;
    }
}
