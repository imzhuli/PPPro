#include "./service_common.hpp"

#include <fstream>
#include <mutex>

/// @brief Common code in ////////

static xIoContext ServiceIoContextInstance;

xRuntimeEnv  RuntimeEnv                 = {};
xLogger *    Logger                     = nullptr;
xLogger *    AuditLogger                = nullptr;
xIoContext * ServiceIoContext           = nullptr;
uint64_t     ServiceIoLoopOnceTimeoutMS = 10;
xTicker      ServiceTicker              = {};
xRunState    ServiceRunState            = {};

static void InitLogger() {
    RuntimeAssert(!Logger);
    Logger = new xBaseLogger();
    RuntimeAssert(static_cast<xBaseLogger *>(Logger)->Init(std::string(RuntimeEnv.DefaultLoggerFilePath).c_str(), false));
    Logger->SetLogLevel(eLogLevel::Debug);

    RuntimeAssert(!AuditLogger);
    AuditLogger = new xBaseLogger();
    RuntimeAssert(static_cast<xBaseLogger *>(AuditLogger)->Init(std::string(RuntimeEnv.DefaultAuditLoggerFilePath).c_str(), false));
}

static void CleanLogger() {
    RuntimeAssert(AuditLogger);
    static_cast<xBaseLogger *>(AuditLogger)->Clean();
    delete Steal(AuditLogger);

    RuntimeAssert(Logger);
    static_cast<xBaseLogger *>(Logger)->Clean();
    delete Steal(Logger);
}

static auto Instance = (xRuntimeEnvGuard *){};
static auto EnvMutex = std::mutex();

xRuntimeEnvGuard::xRuntimeEnvGuard(int argc, char ** argv) {
    auto G = std::lock_guard(EnvMutex);
    RuntimeAssert(!Instance);

    RuntimeEnv = xRuntimeEnv::FromCommandLine(argc, argv);
    InitLogger();
    ServiceIoContext = &ServiceIoContextInstance;
    RuntimeAssert(ServiceIoContext->Init());
    RuntimeAssert(ServiceRunState.Start());
    Instance = this;
}

xRuntimeEnvGuard::~xRuntimeEnvGuard() {
    auto G = std::lock_guard(EnvMutex);
    RuntimeAssert(this == Instance);

    ServiceRunState.Finish();
    Steal(ServiceIoContext)->Clean();
    CleanLogger();
    Reset(RuntimeEnv);
    Reset(Instance);
}

xRuntimeEnv * xRuntimeEnvGuard::operator->() const {
    RuntimeAssert(Instance);
    return &RuntimeEnv;
}

/////////////////////////

bool xServiceRequestContextPool::Init(size_t PoolSize) {
    return Pool.Init(PoolSize);
}

void xServiceRequestContextPool::Clean() {
    Pool.Clean();
    assert(TimeoutList.IsEmpty());
}

auto xServiceRequestContextPool::Acquire(xVariable RequestContext, xVariable RequestContextEx) -> const xServiceRequestContext * {
    auto Id = Pool.Acquire();
    if (!Id) {
        return nullptr;
    }
    auto & N             = Pool[Id];
    N.RequestId          = Id;
    N.RequestTimestampMS = ServiceTicker();
    TimeoutList.AddTail(N);

    N.RequestContext   = RequestContext;
    N.RequestContextEx = RequestContextEx;

    return &N;
}

auto xServiceRequestContextPool::CheckAndGet(uint64_t RequestId) -> const xServiceRequestContext * {
    return Pool.CheckAndGet(RequestId);
}

void xServiceRequestContextPool::Release(const xServiceRequestContext * RCP) {
    assert(Pool.CheckAndGet(RCP->RequestId) == RCP);
    Pool.Release(RCP->RequestId);
}
