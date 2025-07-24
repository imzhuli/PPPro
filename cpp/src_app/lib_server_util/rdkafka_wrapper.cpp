#include "./rdkafka_wrapper.hpp"

#include <typeinfo>

/*
TODO
    在大量发送消息后. 进行重置producer, 这里要考虑线程同步. 以及回收旧对象前的flush和超时操作

*/

/* Callback object */

class xKfkDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
    void dr_cb(RdKafka::Message & message) {
        cout << "CallbackThread: " << std::this_thread::get_id() << endl;
        if (message.err()) {
            ++TotalFailure;
            std::cerr << "消息发送失败: " << message.errstr() << std::endl;
        } else {
            ++TotalSuccess;
            std::cout << "消息发送成功，偏移量: " << message.offset() << ", 延迟: " << message.latency() << std::endl;
        }
    }

    void Reset() {
        xel::Reset(TotalSuccess);
        xel::Reset(TotalFailure);
    }

private:
    size_t TotalSuccess = 0;
    size_t TotalFailure = 0;
};
static xKfkDeliveryReportCb KfkCB;

/**/

bool xKfkProducer::Init(const std::string & Topic, const std::map<std::string, std::string> & KafkaParams) {
    KfkConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    if (!KfkConf) {
        X_DEBUG_PRINTF("Failed to create kafka config");
        return false;
    }
    auto ConfCleaner = xScopeGuard([this] { delete Steal(KfkConf); });

    KfkToipcName          = Topic;
    auto TopicNameCleaner = xScopeGuard([this] { Reset(KfkToipcName); });

    std::string errstr;
    for (auto & [K, V] : KafkaParams) {
        if (RdKafka::Conf::CONF_OK != KfkConf->set(K, V, errstr)) {
            X_DEBUG_PRINTF("failed to set kafka producer param: %s -> %s, error=%s", K.c_str(), V.c_str(), errstr.c_str());
            return false;
        }
    }
    if (RdKafka::Conf::CONF_OK != KfkConf->set("dr_cb", &KfkCB, errstr)) {
        X_DEBUG_PRINTF("failed to set kafka producer param: %s, error=%s", "dr_cb", errstr.c_str());
        return false;
    }

    auto FirstCreate = CreateProducer();
    if (!FirstCreate) {
        X_DEBUG_PRINTF("Failed to create producer with topic");
        return false;
    }

    ConfCleaner.Dismiss();
    TopicNameCleaner.Dismiss();

    RuntimeAssert(RunState.Start());
    PollThread = std::thread([this] { Poll(); });

    X_DEBUG_PRINTF("done");
    return true;
}

void xKfkProducer::Clean() {
    RunState.Stop();
    PollThread.join();
    Reset(PollThread);
    RunState.Finish();

    DestroyProducer();
    auto ConfCleaner      = xScopeGuard([this] { delete Steal(KfkConf); });
    auto TopicNameCleaner = xScopeGuard([this] { Reset(KfkToipcName); });
    X_DEBUG_PRINTF("done");
}

bool xKfkProducer::CreateProducer() {
    assert(!Producer.KfkProducer && !Producer.KfkTopic);
    Producer = CreateNativeProducer();
    if (!Producer.KfkProducer) {
        return false;
    }
    return true;
}

void xKfkProducer::DestroyProducer() {
    DestroyNativeProducer(std::move(Producer));
}

auto xKfkProducer::CreateNativeProducer() -> xTopicProducer {
    auto Ret    = xTopicProducer{};
    auto errstr = std::string();

    Ret.KfkProducer = RdKafka::Producer::create(KfkConf, errstr);
    if (!Ret.KfkProducer) {
        X_DEBUG_PRINTF("error=%s", errstr.c_str());
        return {};
    }
    auto KPG = xScopeGuard([&] { delete Steal(Ret.KfkProducer); });

    Ret.KfkTopic = RdKafka::Topic::create(Ret.KfkProducer, KfkToipcName, NULL, errstr);
    if (!Ret.KfkTopic) {
        X_DEBUG_PRINTF("error=%s", errstr.c_str());
        return {};
    }
    auto KTG = xScopeGuard([&] { delete Steal(Ret.KfkTopic); });

    KPG.Dismiss();
    KTG.Dismiss();
    return Ret;
}

void xKfkProducer::DestroyNativeProducer(xTopicProducer && TP) {
    assert(TP.KfkProducer && TP.KfkTopic);
    auto KPG = xScopeGuard([&] { delete Steal(TP.KfkProducer); });
    auto KTG = xScopeGuard([&] { delete Steal(TP.KfkTopic); });
    return;
}

void xKfkProducer::CheckAndRecreateProducer() {
    Pure();
}

bool xKfkProducer::Post(const std::string & Key, const void * DataPtr, const size_t Size) {
    RdKafka::ErrorCode resp = Producer.KfkProducer->produce(
        Producer.KfkTopic,               // 主题对象
        RdKafka::Topic::PARTITION_UA,    // 分区（PARTITION_UA 表示自动分配）
        RdKafka::Producer::RK_MSG_COPY,  // 消息复制策略
        (char *)(DataPtr),               // 消息内容
        Size,                            // 消息长度
        (Key.empty() ? nullptr : &Key),  // 键（可选）
        NULL                             // 消息头（可选）
    );
    if (resp != RdKafka::ERR_NO_ERROR) {
        X_DEBUG_PRINTF("failed to post payload");
        return false;
    }
    return true;
}

void xKfkProducer::Flush() {
    if (!Producer.KfkProducer) {
        return;
    }
    Producer.KfkProducer->flush(0);
}

void xKfkProducer::Poll() {
    while (RunState) {
        Producer.KfkProducer->poll(100);
    }
}
