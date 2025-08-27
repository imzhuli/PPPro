#include "./connection.hpp"

#include <pp_common/_.hpp>

#define OP(x) OS << #x << ": " << (x) << endl

std::string xPR_ConnectionStateNotify::ToString() const {

    auto OS = std::ostringstream();

    OP(ProxySideConnectionId);
    OP(NewState);
    OP(RelaySideContextId);
    OP(TotalUploadedBytes);
    OP(TotalDumpedBytes);

    return OS.str();
}

#undef OP