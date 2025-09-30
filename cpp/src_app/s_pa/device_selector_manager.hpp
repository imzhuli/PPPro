#pragma once
#include "../lib_client/device_selector_client.hpp"

#include <pp_common/_.hpp>

extern void InitDeviceSelector();
extern void CleanDeviceSelector();
extern void TickDeviceSelector(uint64_t NowMS);
extern bool PostDeviceSelectorRequest(uint64_t SourceRequestId, const xDeviceSelectorOptions & Ops);