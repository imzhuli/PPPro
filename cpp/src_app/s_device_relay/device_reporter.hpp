#pragma once
#include "./device_manager.hpp"

#include <pp_common/_.hpp>

extern void ReportKeepAliveDevice(const xDR_DeviceContext * PDC);
extern void ReportDeviceDrop(const xDR_DeviceContext * PDC);
