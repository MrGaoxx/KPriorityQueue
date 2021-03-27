/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_PRIORITY_QUEUE_H
#define PRIORITY_Q_PRIORITY_QUEUE_H

#include <queue>

#include "rate_limiter.h"
#include "utils.h"

namespace priority_q {
template <class E, uint64_t (*F)(const E&)>
class PriorityQueue {
   public:
    PriorityQueue(Priority prio = 0);
    void Enqueue(const Element<E, F>& element);
    void Enqueue(const Element<E, F>&& element);
    bool CanDequeue();
    Element<E, F> Dequeue();
    void AddRateLimiter(RateLimiterBase<E, F>* limiter);
    uint64_t GetAvailableTime();
    uint64_t size();
    uint32_t length();
    void set_prio(Priority prio);

   private:
    Priority m_prio;
    uint64_t m_size;
    std::list<RateLimiterBase<E, F>*> m_limiters;
    std::queue<Element<E, F>> m_elements;
};

template <class E, uint64_t (*F)(const E&)>
PriorityQueue<E, F>::PriorityQueue(Priority prio) : m_prio(prio), m_size(0), m_limiters(), m_elements(){};

template <class E, uint64_t (*F)(const E&)>
void PriorityQueue<E, F>::Enqueue(const Element<E, F>& _element) {
    for (auto& limiter : m_limiters) {
        limiter->EnqueueTrigger(_element);
    }
    m_elements.push(_element);
    m_size += F(_element.m_element);
    return;
};

template <class E, uint64_t (*F)(const E&)>
void PriorityQueue<E, F>::Enqueue(const Element<E, F>&& _element) {
    for (auto& limiter : m_limiters) {
        limiter->EnqueueTrigger(_element);
    }
    m_size += F(_element.m_element);
    m_elements.push(std::move(_element));
    return;
};

template <class E, uint64_t (*F)(const E&)>
Element<E, F> PriorityQueue<E, F>::Dequeue() {
    Element<E, F> rval = m_elements.front();

    for (auto& limiter : m_limiters) {
        limiter->DequeueTrigger(rval);
    }

    KASSERT(m_size >= F(rval.m_element));
    m_size -= F(rval.m_element);
    m_elements.pop();
    return std::move(rval);
};  // namespace priority_q

template <class E, uint64_t (*F)(const E&)>
bool PriorityQueue<E, F>::CanDequeue() {
    if (m_elements.empty()) {
        return false;
    }

    for (auto& limiter : m_limiters) {
        if (limiter->IsLimited()) {
            return false;
        }
    }
    return true;
};

template <class E, uint64_t (*F)(const E&)>
uint64_t PriorityQueue<E, F>::GetAvailableTime() {
    uint64_t time = UINT64_MAX;
    for (auto& limiter : m_limiters) {
        uint64_t time_limiter = limiter->GetAvailableTime();
        time = (time > time_limiter) ? time_limiter : time;
    }
    return time;
};

template <class E, uint64_t (*F)(const E&)>
uint64_t PriorityQueue<E, F>::size() {
    return m_size;
}

template <class E, uint64_t (*F)(const E&)>
uint32_t PriorityQueue<E, F>::length() {
    return m_elements.size();
}

template <class E, uint64_t (*F)(const E&)>
void PriorityQueue<E, F>::set_prio(Priority prio) {
    m_prio = prio;
};

}  // namespace priority_q

#endif /* PRIORITY_Q_PRIORITY_QUEUE_H */