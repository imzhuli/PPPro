#pragma once
#include "./_.hpp"

using namespace xel;

#include <atomic>
#include <cinttypes>
#include <iostream>
#include <mutex>
#include <string>
#include <tuple>

// clang-format off

#define CASE_PRINT(x) case x: X_DEBUG_PRINTF("%s", X_STRINGIFY(x)); break

template<typename T>
std::unique_ptr<T> P2U(T * Ptr) { return std::unique_ptr<T>(Ptr); }

std::string DebugSign(const void * DataPtr, size_t Size);
static inline std::string DebugSign(const std::string_view& V) { return DebugSign(V.data(), V.size()); }

extern uint32_t HashString(const char * S);
extern uint32_t HashString(const char * S, size_t Len);
extern uint32_t HashString(const std::string & S);

// clang-format on
