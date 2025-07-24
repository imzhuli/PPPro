#include "./_.hpp"

uint64_t         ServerId = {};
xNetAddress      ExportProxyAddress;
eRelayServerType ServerType;
xNetAddress      ExportDeviceCtrlAddress;
xNetAddress      ExportDeviceDataAddress;

#define O(x) OS << #x << "=" << (x) << ' '
std::string xRelayServerInfoBase::ToString() const {
    auto OS = std::ostringstream();
    O(ServerId);
    O(ExportProxyAddress.ToString());
    O((unsigned)ServerType);
    O(ForcedPoolId);
    O(StartupTimestampMS);
    O(ExportDeviceCtrlAddress.ToString());
    O(ExportDeviceDataAddress.ToString());
    return OS.str();
}
