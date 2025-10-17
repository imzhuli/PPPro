#pragma once
#include "../lib_client/device_selector_client.hpp"

#include <pp_common/_.hpp>

/**
    @brief InitUdpChannelManager
    AddressMapping: bind address (local address) -> export address
 */
bool InitUdpChannelManager();
void CleanUdpChanneManager();

struct xPA_UdpChannelBinding {
    xNetAddress ExportBindingAddress;
};

xPA_UdpChannelBinding * CreateUdpChannel(void * Owner);
void                    DestroyUdpChannel(xPA_UdpChannelBinding *);
