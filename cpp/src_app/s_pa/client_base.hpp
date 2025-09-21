#pragma once
#include <pp_common/_.hpp>

enum xPA_ClientState {

    CS_C,     // challenge, protocol unknown
    CS_S_C,   // s5 challenge
    CS_S_WA,  // wait for auth info
    CS_S_WT,  // wait for target
    CS_S_TC,  // tcp connection

};
