#include "./device_selector_manager.hpp"

#include "./_global.hpp"

static xDeviceSelectorClient DeviceSelectorClient;

static void DispatchDeviceSelectorResult(uint64_t RequestContextId, const xDeviceSelectorResult & Result);

void InitDeviceSelector() {
    RuntimeAssert(DeviceSelectorClient.Init(ServiceIoContext, ConfigServerListDownloadAddress));
    DeviceSelectorClient.OnDeviceSelectResultCallback = DispatchDeviceSelectorResult;
}

void CleanDeviceSelector() {
    DeviceSelectorClient.Clean();
}

void TickDeviceSelector(uint64_t NowMS) {
    DeviceSelectorClient.Tick(NowMS);
}

bool PostDeviceSelectorRequest(uint64_t SourceRequestId, const xDeviceSelectorOptions & Ops) {
    return DeviceSelectorClient.Request(SourceRequestId, Ops);
}

void DispatchDeviceSelectorResult(uint64_t RequestContextId, const xDeviceSelectorResult & Result) {
}
