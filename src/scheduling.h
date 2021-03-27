/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_SHCEDULING_H
#define PRIORITY_Q_SHCEDULING_H

#include <queue>

#include "priority_queues.h"

namespace priority_q {

template <class E, uint64_t (*F)(const E&)>
class RRScheduling : public Scheduling<E, F> {
   public:
    RRScheduling(PriorityQueues<E, F>*);
    virtual Priority GetNextPriority() override;
    virtual void EnqueueTrigger(Priority piro) override;
    virtual void DequeueTrigger(Priority piro) override;

   private:
    Priority m_last_prio;
};

template <class E, uint64_t (*F)(const E&)>
class SPScheduling : public Scheduling<E, F> {
   public:
    using Scheduling<E, F>::Scheduling;
    virtual Priority GetNextPriority() override;
    virtual void EnqueueTrigger(Priority piro) override;
    virtual void DequeueTrigger(Priority piro) override;
};

using WFQTokens = std::array<uint8_t, kMaxPriorities>;

template <class E, uint64_t (*F)(const E&)>
class WFQScheduling : public Scheduling<E, F> {
   public:
    WFQScheduling(PriorityQueues<E, F>*);
    virtual Priority GetNextPriority() override;
    virtual void EnqueueTrigger(Priority piro) override;
    virtual void DequeueTrigger(Priority piro) override;

    void set_tokens(WFQTokens&& token);

   private:
    Priority m_last_prio;
    uint8_t m_last_token;
    WFQTokens m_tokens;
};

template <class E, uint64_t (*F)(const E&)>
RRScheduling<E, F>::RRScheduling(PriorityQueues<E, F>* priority_queues) : Scheduling<E, F>(SchedulePolicy::RR, priority_queues), m_last_prio(0) {}

template <class E, uint64_t (*F)(const E&)>
Priority RRScheduling<E, F>::GetNextPriority() {
    Priority prio = m_last_prio;
    for (Priority prio_count = 0; prio_count < Scheduling<E, F>::m_max_prio; ++prio_count) {
        if (Scheduling<E, F>::m_queues->GetQueue(prio)->CanDequeue()) {
            m_last_prio = prio;
            return prio;
        }
        prio = (prio + 1) % Scheduling<E, F>::m_max_prio;
    };
    return kNullPriority;
}
template <class E, uint64_t (*F)(const E&)>
void RRScheduling<E, F>::EnqueueTrigger(Priority prio) {
    return;
}
template <class E, uint64_t (*F)(const E&)>
void RRScheduling<E, F>::DequeueTrigger(Priority prio) {
    assert(prio == m_last_prio);
    m_last_prio = (m_last_prio + 1) % Scheduling<E, F>::m_max_prio;
}

template <class E, uint64_t (*F)(const E&)>
Priority SPScheduling<E, F>::GetNextPriority() {
    for (Priority priority = 0; priority < Scheduling<E, F>::m_max_prio; ++priority) {
        if (Scheduling<E, F>::m_queues->GetQueue(priority)->CanDequeue()) {
            return priority;
        }
    };
    return kNullPriority;
}
template <class E, uint64_t (*F)(const E&)>
void SPScheduling<E, F>::EnqueueTrigger(Priority prio) {
    return;
}
template <class E, uint64_t (*F)(const E&)>
void SPScheduling<E, F>::DequeueTrigger(Priority prio) {
    return;
}

template <class E, uint64_t (*F)(const E&)>
WFQScheduling<E, F>::WFQScheduling(PriorityQueues<E, F>* priority_queues)
    : Scheduling<E, F>(SchedulePolicy::WFQ, priority_queues), m_last_prio(0), m_last_token(0) {}
template <class E, uint64_t (*F)(const E&)>
void WFQScheduling<E, F>::set_tokens(WFQTokens&& token) {
    m_tokens = std::move(token);
};
template <class E, uint64_t (*F)(const E&)>
Priority WFQScheduling<E, F>::GetNextPriority() {
    Priority prio = m_last_prio;
    for (Priority prio_count = 0; prio_count < Scheduling<E, F>::m_max_prio; ++prio_count) {
        if (Scheduling<E, F>::m_queues->GetQueue(prio)->CanDequeue()) {
            m_last_prio = prio;
            return prio;
        }
        prio = (prio + 1) % Scheduling<E, F>::m_max_prio;
    };
    return kNullPriority;
}
template <class E, uint64_t (*F)(const E&)>
void WFQScheduling<E, F>::EnqueueTrigger(Priority prio) {
    return;
}
template <class E, uint64_t (*F)(const E&)>
void WFQScheduling<E, F>::DequeueTrigger(Priority prio) {
    assert(prio == m_last_prio);
    ++m_last_token;
    if (unlikely(m_tokens[m_last_prio] == m_last_token)) {
        m_last_token = 0;
        m_last_prio = (m_last_prio + 1) % Scheduling<E, F>::m_max_prio;
    }
}

}  // namespace priority_q

#endif /* PRIORITY_Q_SHCEDULING_H */