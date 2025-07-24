#include "./connection.hpp"

#include <pp_common/base.hpp>

#define OP(x) OS << #x << ": " << (x) << endl

std::string xPR_ConnectionStateNotify::ToString() const {

    auto OS = std::ostringstream();

    OP(ProxySideConnectionId);
    OP(NewState);
    OP(RelaySideConnectionId);
    OP(TotalUploadedBytes);
    OP(TotalDumpedBytes);

    return OS.str();
}

#undef OP