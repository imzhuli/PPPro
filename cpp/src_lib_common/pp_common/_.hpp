#pragma once
#include <config/config.hpp>
#include <core/core_min.hpp>
#include <core/core_os.hpp>
#include <core/core_stream.hpp>
#include <core/core_time.hpp>
#include <core/executable.hpp>
#include <core/functional.hpp>
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
#include <server_arch/tcp_service.hpp>
#include <server_arch/udp_service.hpp>

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
using xel::xClientConnection;
using xel::xClientPool;
using xel::xCommandLine;
using xel::xConfigLoader;
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
using xel::xSocket;
using xel::xStreamReader;
using xel::xStreamWriter;
using xel::xTcpConnection;
using xel::xTcpServer;
using xel::xTcpService;
using xel::xTcpServiceClientConnectionHandle;
using xel::xTicker;
using xel::xTimer;
using xel::xUdpChannel;
using xel::xUdpService;
using xel::xUdpServiceChannelHandle;

// functions
using xel::Base64Decode;
using xel::Base64Encode;
using xel::BuildPacket;
using xel::Daemonize;
using xel::Delegate;
using xel::FileToLines;
using xel::FileToStr;
using xel::GetTimestampMS;
using xel::HexShow;
using xel::HexToStr;
using xel::Ignore;
using xel::JoinStr;
using xel::Md5;
using xel::Noop;
using xel::Pure;
using xel::RuntimeAssert;
using xel::Split;
using xel::Steal;
using xel::StrToHex;
using xel::StrToHexLower;
using xel::Todo;
using xel::Trim;
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
static constexpr const uint32_t MAX_DEVICE_SELECTOR_COUNT            = 1000;
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

struct xGeoInfoEx : xGeoInfo {
    std::string CountryName;
    std::string CityName;
    std::string ShortCityName;
};

struct xDeviceId final {
    uint64_t RelayServerRuntimeId;
    uint64_t RelayServerSideDeviceId;

    std::strong_ordering operator<=>(const xDeviceId &) const = default;
};

using xDeviceFlag              = uint32_t;
using xDevicePoolId            = uint32_t;
using xDeviceSelectionStrategy = uint16_t;

constexpr const xDeviceFlag DF_NONE        = 0x00;
constexpr const xDeviceFlag DF_ENABLE_TCP4 = 0x01 << 0;
constexpr const xDeviceFlag DF_ENABLE_TCP6 = 0x01 << 1;
constexpr const xDeviceFlag DF_ENABLE_UDP4 = 0x01 << 2;
constexpr const xDeviceFlag DF_ENABLE_UDP6 = 0x01 << 3;

constexpr const xDeviceSelectionStrategy DSS_CLEAR             = 0;
constexpr const xDeviceSelectionStrategy DSS_IPV4              = 0x01u << 0;
constexpr const xDeviceSelectionStrategy DSS_IPV6              = 0x01u << 1;
constexpr const xDeviceSelectionStrategy DSS_UDP               = 0x01u << 2;
constexpr const xDeviceSelectionStrategy DSS_DEVICE_PERSISTENT = 0x01u << 3;
constexpr const xDeviceSelectionStrategy DSS_DEVICE_VOLATILE   = 0x01u << 4;
constexpr const xDeviceSelectionStrategy DSS_REGION_DOWNGRADE  = 0x01u << 5;

struct xDeviceChallengePack {
    uint32_t    Version;
    uint32_t    ChannelId;
    xNetAddress Tcp4Address;
    xNetAddress Tcp6Address;
    xNetAddress Udp4Address;
    xNetAddress Udp6Address;
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
    uint64_t    ServerId         = {};
    xNetAddress ProducerAddress4 = {};
    xNetAddress ObserverAddress4 = {};

    bool operator==(const xRelayInfoDispatcherServerInfo &) const = default;

    std::string ToString() const;
};

enum struct eRelayServerType : uint16_t {
    UNSPECIFIED = 0,
    DEVICE      = 1,
    THIRD       = 2,
    STATIC      = 3,
};

struct xRelayServerInfoBase {
    eRelayServerType ServerType;
    uint64_t         ServerId = {};
    uint64_t         StartupTimestampMS;
    xNetAddress      ExportProxyAddress4;
    xNetAddress      ExportProxyAddress6;

    xNetAddress ExportDeviceAddress4;
    xNetAddress ExportDeviceAddress6;

    uint32_t ForcedPoolId;

    std::string ToString() const;
};

struct xDeviceSelectorDispatcherInfo {
    uint64_t    ServerId = {};
    uint64_t    StartupTimestampMS;
    xNetAddress ExportAddressForClient          = {};
    xNetAddress ExportAddressForServiceProvider = {};
};

struct xDeviceSelectorServerInfo {
    static constexpr const uint16_t MAX_DEVICE_POOL_ID = 128;

    enum eRegionDetailLevel : uint16_t {
        UNSPECIFIED = 0,
        COUNTRY     = 1,
        STATE       = 2,
        CITY        = 3,
        LOCAL_AREA  = 4,
    };

    static constexpr const size_t MAX_STRATEGY_COMBINED_ENABLED_COUNT = 32;

    eRegionDetailLevel       RegionDetailLevel = UNSPECIFIED;
    xDeviceSelectionStrategy StrategyFlags     = DSS_CLEAR;
    xDevicePoolId            DevicePoolId      = 0;
};

struct xClientAuthResult {
    xAuditId    AuditId;
    xCountryId  CountryId;
    xStateId    StateId;
    xCityId     CityId;
    bool        RequireIpv6;
    bool        RequireUdp;
    bool        AlwaysChangeIp;
    bool        PersistentDeviceBinding;
    std::string PAToken;
    std::string ThirdRedirect;
};

struct xAuditAccountInfo {
    uint64_t AuditId           = {};
    uint64_t TotalUploadSize   = {};
    uint64_t TotalDownloadSize = {};
    uint64_t TotalTcpCount     = {};
    uint64_t TotalUdpCount     = {};
};

struct xMessageChannel {
    virtual uint64_t GetInternalId() const { return 0; }
    virtual void *   GetUnderLayeredObject() const { return nullptr; }
    virtual void *   GetUnderLayeredObjectUnchecked() const { return GetUnderLayeredObject(); }
    virtual void     PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const = 0;
    virtual void     PostMessageUnchecked(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const { PostMessage(CmdId, RequestId, Message); }
};

constexpr const struct final {
    void Tick(uint64_t) const {}
} DeadTicker;

namespace __pp_common_detail__ {
    inline void __TickAll__(uint64_t) {  // iteration finishes here
    }
    template <typename T, typename... TOthers>
    inline void __TickAll__(uint64_t NowMS, T && First, TOthers &&... Others) {
        std::forward<T>(First).Tick(NowMS);
        __TickAll__(NowMS, std::forward<TOthers>(Others)...);
    }

}  // namespace __pp_common_detail__

template <typename... T>
inline void TickAll(uint64_t NowMS, T &&... All) {
    __pp_common_detail__::__TickAll__(NowMS, std::forward<T>(All)...);
}

template <typename T>
class xSingleton {
    static std::atomic<xSingleton *> InstnacePtr;

protected:
    inline xSingleton() {
        xSingleton * Expected = nullptr;
        RuntimeAssert(InstnacePtr.compare_exchange_strong(Expected, this), "mutiple instance of singlton type");
    }
    inline ~xSingleton() {
        xSingleton * Expected = this;
        RuntimeAssert(InstnacePtr.compare_exchange_strong(Expected, nullptr), "mutiple instance destroyed of singlton type");
    }
    inline xSingleton(xSingleton &&) = delete;
};
template <typename T>
std::atomic<xSingleton<T> *> xSingleton<T>::InstnacePtr = nullptr;

// clang-format off

static inline uint32_t High32(uint64_t U) { return (uint32_t)(U >> 32); }
static inline uint32_t Low32(uint64_t U)  { return (uint32_t)(U); }
static inline uint64_t Make64(uint32_t H32, uint32_t L32) { return (static_cast<uint64_t>(H32) << 32) + L32; }

static inline uint16_t High16(uint64_t U) { return (uint16_t)(U >> 48); }
static inline uint64_t Low48(uint64_t U)  { return U & 0x0000'FFFF'FFFF'FFFFu; }
static inline uint64_t Make64_H16L48(uint16_t H16, uint64_t L48) { return (static_cast<uint64_t>(H16) << 48) + L48; }

// clang-format on

// clang-format off

template<typename T>
std::unique_ptr<T> P2U(T * && Ptr) { return std::unique_ptr<T>(std::move(Ptr)); }

extern uint32_t HashString(const char * S);
extern uint32_t HashString(const char * S, size_t Len);
extern uint32_t HashString(const std::string_view & S);

extern std::string AppSign(uint64_t Timestamp, const std::string & SecretKey, const void * DataPtr, size_t Size);
static inline std::string AppSign(uint64_t Timestamp, const std::string & SecretKey, const std::string_view& V) { return AppSign(Timestamp, SecretKey, V.data(), V.size()); }

extern bool ValidateAppSign(const std::string & Sign, const std::string & SecretKey, const void * DataPtr, size_t Size);
static inline bool ValidateAppSign(const std::string & Sign, const std::string & SecretKey, const std::string_view& V) { return ValidateAppSign(Sign, SecretKey, V.data(), V.size()); }

extern std::string SignAndPackAddress(uint64_t Timestamp, const std::string & SecretKey, const xNetAddress & Address);
extern xNetAddress ExtractAddressFromPack(const std::string & SignedIp, const std::string & SecretKey);

// clang-format on
inline std::ostream & operator<<(std::ostream & OS, const xNetAddress & Address) {
    OS << Address.ToString();
    return OS;
}

#define X_INSTANCE   auto X_VAR =
#define X_AT_EXIT(v) auto X_CONCAT_FORCE_EXPAND(__X_AtExit__, __LINE__) = ::xel::xScopeGuard(v);
