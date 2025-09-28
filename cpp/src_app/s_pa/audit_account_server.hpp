#pragma once
#include "../lib_utils/all.hpp"

extern void InitAuditAccountService();
extern void CleanAuditAccountService();
extern void TickAuditAccountService(uint64_t NowMS);
