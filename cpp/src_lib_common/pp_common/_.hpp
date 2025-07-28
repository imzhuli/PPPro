#pragma once
#include <core/core_min.hpp>
#include <core/core_os.hpp>
#include <core/core_stream.hpp>
#include <core/core_time.hpp>
#include <core/executable.hpp>
#include <core/indexed_storage.hpp>
#include <core/list.hpp>
#include <core/logger.hpp>
#include <core/memory.hpp>
#include <core/memory_pool.hpp>
#include <core/optional.hpp>
#include <core/string.hpp>
#include <crypto/base64.hpp>
#include <crypto/md5.hpp>
#include <network/net_address.hpp>
#include <network/tcp_connection.hpp>
#include <network/tcp_server.hpp>
#include <network/udp_channel.hpp>
#include <object/object.hpp>
#include <server_arch/client.hpp>
#include <server_arch/client_pool.hpp>
#include <server_arch/message.hpp>
#include <server_arch/service.hpp>

//
#include <cinttypes>

//
using namespace xel::common;
using namespace std::chrono_literals;

// consts
using xel::InvalidDataSize;
using xel::MaxPacketPayloadSize;
using xel::MaxPacketSize;
using xel::PacketHeaderSize;

// type-defs
using xel::eLogLevel;
using xel::xAbstract;
using xel::xBaseLogger;
using xel::xBinaryMessage;
using xel::xClient;
using xel::xClientPool;
using xel::xCommandLine;
using xel::xIndexedStorage;
using xel::xIndexId;
using xel::xIndexIdPool;
using xel::xIoContext;
using xel::xList;
using xel::xListNode;
using xel::xLogger;
using xel::xMd5Result;
using xel::xMemoryPool;
using xel::xMemoryPoolOptions;
using xel::xNetAddress;
using xel::xObjectIdManager;
using xel::xObjectIdManagerMini;
using xel::xOptional;
using xel::xPacket;
using xel::xPacketCommandId;
using xel::xPacketHeader;
using xel::xPacketRequestId;
using xel::xResourceGuard;
using xel::xScopeCleaner;
using xel::xScopeGuard;
using xel::xService;
using xel::xSocket;
using xel::xStreamReader;
using xel::xStreamWriter;
using xel::xTcpConnection;
using xel::xTcpServer;
using xel::xTicker;
using xel::xUdpChannel;

// functions
using xel::Base64Decode;
using xel::Base64Encode;
using xel::BuildPacket;
using xel::Daemonize;
using xel::GetTimestampMS;
using xel::HexShow;
using xel::HexToStr;
using xel::Md5;
using xel::Pure;
using xel::RuntimeAssert;
using xel::Split;
using xel::Steal;
using xel::StrToHex;
using xel::Todo;
using xel::Unreachable;
using xel::WriteMessage;
using xel::ZeroFill;

// std-lib:
#include <functional>
#include <iostream>
using std::bind;
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::function;

// min_defs:
using xVersion = uint32_t;

using xGroupId      = uint32_t;
using xServerId     = uint32_t;
using xAccountId    = uint64_t;
using xTerminalId   = uint64_t;
using xConnectionId = uint64_t;

using xContinentId = uint32_t;
using xCountryId   = uint32_t;
using xStateId     = uint32_t;
using xCityId      = uint32_t;

using xServerRouteId = uint16_t;  // 强行划分的服务器路径归属 id
using xSourcePoolId  = uint16_t;  //
using xAuditId       = uint64_t;

using xServerRuntimeKey = uint64_t;  // 由配置中心分配的动态服务器ID, 由序列和随机数组成

// min static constants

static constexpr const size_t   DEFAULT_MAX_SERVER_CONNECTIONS       = 4096;
static constexpr const uint32_t MAX_DEVICE_RELAY_SERVER_SUPPORTED    = 4096;  // IdManagerMini::MaxObjectId
static constexpr const uint32_t MAX_AUTH_CACHE_SERVER_COUNT          = 75;
static constexpr const uint32_t MAX_DEVICE_STATE_RELAY_SERVER_COUNT  = 75;
static constexpr const uint32_t MAX_AUDIT_DEVICE_SERVER_COUNT        = 75;
static constexpr const uint32_t MAX_AUDIT_ACCOUNT_SERVER_COUNT       = 75;
static constexpr const uint32_t MAX_DEVICE_SELECTOR_DISPATCHER_COUNT = 75;
static constexpr const uint32_t MAX_BACKEND_SERVER_COUNT             = 75;

enum struct eServerType : uint32_t {
    UNSPECIFIED = 0x00,

    // main service
    CONFIG_CENTER              = 0x01,  // CC
    DEVICE_RELAY               = 0x02,  // DR
    DEVICE_STATE_RELAY         = 0x03,  // DSR
    DEVICE_SELECTOR            = 0x04,  // DS
    DEVICE_SELECTOR_DISPATCHER = 0x05,  // DSD
    THIRD_RELAY                = 0x06,  // TR
    STATIC_RELAY_TERMINAL      = 0x07,  // SRT
    RELAY_INFO_DISPATCHER      = 0x08,  // RID
    PROXY_ACCESS               = 0x09,  // PA
    AUTH_CACHE                 = 0x0A,  // AC
    AUDIT_DEVICE_CACHE         = 0x0B,  // ADC
    AUDIT_ACCOUNT_CACHE        = 0x0C,  // AAC

};

static constexpr xContinentId CID_UNSPECIFIC    = 0x00;
static constexpr xContinentId CID_ASIA          = 0x01;
static constexpr xContinentId CID_EUROP         = 0x02;
static constexpr xContinentId CID_NORTH_AMERICA = 0x03;
static constexpr xContinentId CID_SOUTH_AMERICA = 0x04;
static constexpr xContinentId CID_AFRICA        = 0x05;
static constexpr xContinentId CID_OCEANIA       = 0x06;
static constexpr xContinentId CID_ANTARCTIC     = 0x07;

static constexpr const xContinentId WildContinentId = xContinentId(-1);

struct xGeoInfo {
    xCountryId CountryId;
    xStateId   StateId;
    xCityId    CityId;
};

struct xGeoInfoEx {
    xGeoInfo RegionId;

    std::string CountryName;
    std::string CityName;
    std::string ShortCityName;
};

struct xServerInfo {
    uint64_t    ServerId = {};
    xNetAddress Address  = {};

    bool operator==(const xServerInfo &) const = default;
};

struct xDeviceStateRelayServerInfo {
    uint64_t    ServerId        = {};
    xNetAddress ProducerAddress = {};
    xNetAddress ObserverAddress = {};

    bool operator==(const xDeviceStateRelayServerInfo &) const = default;
};

struct xRelayInfoDispatcherServerInfo {
    uint64_t    ServerId        = {};
    xNetAddress ProducerAddress = {};
    xNetAddress ObserverAddress = {};

    bool operator==(const xRelayInfoDispatcherServerInfo &) const = default;
};

enum struct eRelayServerType : uint16_t {
    UNSPECIFIED = 0,
    DEVICE      = 1,
    THIRD       = 2,
    STATIC      = 3,
};

struct xRelayServerInfoBase {
    uint64_t         ServerId = {};
    uint64_t         StartupTimestampMS;
    xNetAddress      ExportProxyAddress;
    eRelayServerType ServerType;
    uint64_t         Flags;
    uint64_t         FlagsEx;
    uint32_t         ForcedPoolId;

    xNetAddress ExportDeviceCtrlAddress;
    xNetAddress ExportDeviceDataAddress;
    xNetAddress ExportStaticIpAddress;
    xNetAddress ExportStaticUdpAddress;

    std::string ToString() const;
};

struct xDeviceSelectorDispatcherInfo {
    uint64_t    ServerId = {};
    uint64_t    StartupTimestampMS;
    xNetAddress ExportAddressForClient          = {};
    xNetAddress ExportAddressForServiceProvider = {};
};

struct xClientAuthResult {
    xAuditId    AuditId;
    xCountryId  CountryId;
    xStateId    StateId;
    xCityId     CityId;
    bool        RequireIpv6;
    bool        RequireUdp;
    bool        AutoChangeIp;
    std::string PAToken;
};

struct xAuditAccountInfo {
    uint64_t AuditId           = {};
    uint64_t TotalUploadSize   = {};
    uint64_t TotalDownloadSize = {};
    uint64_t TotalTcpCount     = {};
    uint64_t TotalUdpCount     = {};
};

struct xMessagePoster {
    virtual void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) = 0;
};

// clang-format off

namespace __pp_common_detail__ {
    template <typename T>
    inline void __TickOne__(uint64_t NowMS, T & Target) {
        Target.Tick(NowMS);
    }
    inline void __TickAll__(uint64_t) {  // iteration finishes here
    }
    template <typename T, typename... TOthers>
    inline void __TickAll__(uint64_t NowMS, T & First, TOthers &... Others) {
        __TickOne__(NowMS, First);
        __TickAll__(NowMS, Others...);
    }

}  // namespace __pp_common_detail__

template <typename... T>
inline void TickAll(uint64_t NowMS, T &... All) {
    __pp_common_detail__::__TickAll__(NowMS, All...);
}

static inline uint32_t High32(uint64_t U) { return (uint32_t)(U >> 32); }
static inline uint32_t Low32(uint64_t U)  { return (uint32_t)(U); }
static inline uint64_t Make64(uint32_t H32, uint32_t L32) { return (static_cast<uint64_t>(H32) << 32) + L32; }

static inline uint16_t High16(uint64_t U) { return (uint16_t)(U >> 48); }
static inline uint64_t Low48(uint64_t U)  { return U & 0x0000'FFFF'FFFF'FFFFu; }
static inline uint64_t Make64_H16L48(uint16_t H16, uint64_t L48) { return (static_cast<uint64_t>(H16) << 48) + L48; }

// clang-format on

#define X_CONCAT(a, b)              a##b
#define X_CONCAT_FORCE_EXPAND(a, b) X_CONCAT(a, b)

#ifndef X_GUARD
#define X_GUARD(...)                                                                        \
    auto X_CONCAT_FORCE_EXPAND(__X_Guard__, __LINE__) = ::xel::xResourceGuard(__VA_ARGS__); \
    ::xel::RuntimeAssert(X_CONCAT_FORCE_EXPAND(__X_Guard__, __LINE__))
#endif

#ifndef X_COND_GUARD
#define X_COND_GUARD(cond, ...)                                                                                                                     \
    auto X_CONCAT_FORCE_EXPAND(__X_Cond__, __LINE__)  = (bool)(cond);                                                                               \
    auto X_CONCAT_FORCE_EXPAND(__X_Guard__, __LINE__) = ::xel::xConditionalResourceGuard(X_CONCAT_FORCE_EXPAND(__X_Cond__, __LINE__), __VA_ARGS__); \
    ::xel::RuntimeAssert(!X_CONCAT_FORCE_EXPAND(__X_Cond__, __LINE__) || X_CONCAT_FORCE_EXPAND(__X_Guard__, __LINE__))
#endif
