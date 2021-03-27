/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_PRIORITY_QUEUES_H
#define PRIORITY_Q_PRIORITY_QUEUES_H

#include <map>
#include <queue>

#include "priority_queue.h"

namespace priority_q {
constexpr Priority kMaxPriorities = 8;

const uint64_t kMaxTime = UINT64_MAX;

using PriorityEndian = enum class enum_priority_endian {
    SMALL = 0,  // means that lower priority is larger
    BIG = 1
};
constexpr PriorityEndian kUsingPriorityEndian = PriorityEndian::SMALL;

template <class E, uint64_t (*F)(const E&)>
class PriorityQueues;

using SchedulePolicy = enum class enum_schedule_policy { RR = 0, SP = 1, WFQ = 2 };

template <class E, uint64_t (*F)(const E&)>
class Scheduling {
   public:
    Scheduling(SchedulePolicy, PriorityQueues<E, F>*);
    virtual Priority GetNextPriority() = 0;
    virtual void EnqueueTrigger(Priority piro) = 0;
    virtual void DequeueTrigger(Priority piro) = 0;

   protected:
    SchedulePolicy m_type;
    PriorityQueues<E, F>* m_queues;
    Priority m_max_prio;
};

template <class E, uint64_t (*F)(const E&)>
Scheduling<E, F>::Scheduling(SchedulePolicy policy, PriorityQueues<E, F>* priority_queues)
    : m_type(policy), m_queues(priority_queues), m_max_prio(priority_queues->get_max_prio()) {}

template <class E, uint64_t (*F)(const E&)>
class PriorityQueues {
   public:
    PriorityQueues(Priority max_prio);
    virtual ~PriorityQueues();
    void set_scheduling(Scheduling<E, F>* scheduling);

    int Enqueue(Priority prio, const E& element);
    int Enqueue(Priority prio, const E&& element);
    int Enqueue(Priority prio, const Element<E, F>& element);
    int Enqueue(Priority prio, const Element<E, F>&& element);
    PriorityQueue<E, F>* GetQueue(Priority prio);
    virtual void DequeueTrigger(Priority prio, Element<E, F>& element);
    uint64_t GetAvailableTimeInUs();

    Element<E, F> Dequeue();
    Priority get_max_prio();

    uint64_t GetQueuingSize();
    uint32_t GetQueuingLength();
    uint64_t GetQueuingSize(Priority prio);
    uint32_t GetQueuingLength(Priority prio);

   private:
    Scheduling<E, F>* m_scheduling;
    std::array<PriorityQueue<E, F>, kMaxPriorities> m_queues;
    Priority m_max_prio;
    uint64_t m_size;
    uint64_t m_length;
};

template <class E, uint64_t (*F)(const E&)>
PriorityQueues<E, F>::PriorityQueues(Priority max_prio) : m_scheduling(nullptr), m_max_prio(max_prio), m_size(0), m_length(0) {
    for (int prio = 0; prio < kMaxPriorities; prio++) {
        m_queues[prio].set_prio(prio);
    }
}
template <class E, uint64_t (*F)(const E&)>
PriorityQueues<E, F>::~PriorityQueues() {
    if (likely(m_scheduling)) {
        delete m_scheduling;
        m_scheduling = nullptr;
    }
};

template <class E, uint64_t (*F)(const E&)>
void PriorityQueues<E, F>::set_scheduling(Scheduling<E, F>* scheduling) {
    if (m_scheduling) {
        delete m_scheduling;
    }
    m_scheduling = scheduling;
};

template <class E, uint64_t (*F)(const E&)>
PriorityQueue<E, F>* PriorityQueues<E, F>::GetQueue(Priority prio) {
    return &m_queues[prio];
};

template <class E, uint64_t (*F)(const E&)>
Priority PriorityQueues<E, F>::get_max_prio() {
    return m_max_prio;
};

template <class E, uint64_t (*F)(const E&)>
int PriorityQueues<E, F>::Enqueue(Priority prio, const E& element) {
    return Enqueue(prio, Element<E, F>(prio, element));
};

template <class E, uint64_t (*F)(const E&)>
int PriorityQueues<E, F>::Enqueue(Priority prio, const E&& element) {
    return Enqueue(prio, Element<E, F>(prio, std::forward<E>(element)));
};

template <class E, uint64_t (*F)(const E&)>
int PriorityQueues<E, F>::Enqueue(Priority prio, const Element<E, F>& element) {
    if (unlikely(prio > m_max_prio)) {
        return -EINVAL;
    }
    m_size += element.GetSize();
    ++m_length;
    m_queues[prio].Enqueue(element);
    return 0;
}
template <class E, uint64_t (*F)(const E&)>
int PriorityQueues<E, F>::Enqueue(Priority prio, const Element<E, F>&& element) {
    if (likely(prio <= m_max_prio)) {
        m_scheduling->EnqueueTrigger(prio);
        m_size += element.GetSize();
        ++m_length;
        m_queues[prio].Enqueue(std::move(element));
        return 0;
    } else {
        return -EINVAL;
    }
};

template <class E, uint64_t (*F)(const E&)>
void PriorityQueues<E, F>::DequeueTrigger(Priority prio, Element<E, F>& element){};

template <class E, uint64_t (*F)(const E&)>
Element<E, F> PriorityQueues<E, F>::Dequeue() {
    Priority next_prio = m_scheduling->GetNextPriority();
    Element<E, F> rval;
    rval.m_prio = next_prio;
    if (likely(next_prio != kNullPriority)) {
        rval = m_queues[next_prio].Dequeue();
        m_scheduling->DequeueTrigger(next_prio);
        DequeueTrigger(next_prio, rval);
        KASSERT(m_size >= rval.GetSize());
        m_size -= rval.GetSize();
        --m_length;
    }
    return rval;
};

template <class E, uint64_t (*F)(const E&)>
uint64_t PriorityQueues<E, F>::GetAvailableTimeInUs() {
    uint64_t rval = UINT64_MAX;
    for (int prio = 0; prio < kMaxPriorities; ++prio) {
        uint64_t time_queue = m_queues[prio].GetAvailableTime();
        rval = (rval < time_queue) ? rval : time_queue;
    }
    return rval;
};

template <class E, uint64_t (*F)(const E&)>
uint64_t PriorityQueues<E, F>::GetQueuingSize() {
    return m_size;
};
template <class E, uint64_t (*F)(const E&)>
uint32_t PriorityQueues<E, F>::GetQueuingLength() {
    return m_length;
};
template <class E, uint64_t (*F)(const E&)>
uint64_t PriorityQueues<E, F>::GetQueuingSize(Priority prio) {
    assert(prio <= kMaxPriorities);
    return m_queues[prio].size();
};
template <class E, uint64_t (*F)(const E&)>
uint32_t PriorityQueues<E, F>::GetQueuingLength(Priority prio) {
    assert(prio <= kMaxPriorities);
    return m_queues[prio].length();
};

}  // namespace priority_q

#endif /* PRIORITY_Q_PRIORITY_QUEUES_H */