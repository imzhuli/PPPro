#include "./relay_info.hpp"

#include <sstream>

std::string ToString(const std::vector<xCC_PA_RelayServerInfo> & List) {
    auto OS = std::ostringstream();
    OS << "(" << List.size() << ")";
    OS << "[ ";
    for (auto & I : List) {
        OS << '{' << I.SourcePoolId << ", " << I.ExportProxyAddress.ToString() << '}' << ' ';
    }
    OS << "]";
    return OS.str();
}
