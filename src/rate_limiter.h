/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_RATE_LIMITER_H
#define PRIORITY_Q_RATE_LIMITER_H
#include "pq_timer.h"

namespace priority_q {

using Priority = uint8_t;
constexpr Priority kNullPriority = UINT8_MAX;

template <class E, uint64_t (*F)(const E&)>
class PriorityQueue;

template <class E, uint64_t (*F)(const E&)>
class Element {
   public:
    Element();
    Element(Priority, const E&);
    Element(Priority, const E&&);
    Element(const Element<E, F>&);
    Element(const Element<E, F>&&);
    const Element<E, F>& operator=(const Element<E, F>&);
    const Element<E, F>& operator=(const Element<E, F>&&);
    virtual ~Element();
    uint64_t GetSize() const;
    Priority m_prio;
    E m_element;
};

template <class E, uint64_t (*F)(const E&)>
class RateLimiterBase {
   public:
    RateLimiterBase(PriorityQueueTimer* timer);
    virtual ~RateLimiterBase();
    virtual bool IsLimited() = 0;
    virtual void EnqueueTrigger(const Element<E, F>& enqueue_element) = 0;
    virtual void DequeueTrigger(const Element<E, F>& dequeu_element) = 0;
    virtual uint64_t GetAvailableTime() = 0;
    void set_queue(PriorityQueue<E, F>* queue);

   protected:
    PriorityQueue<E, F>* m_queue;
    PriorityQueueTimer* m_timer;
};

template <class E, uint64_t (*F)(const E&), class L>
class RateLimiter : public RateLimiterBase<E, F> {
   public:
    RateLimiter(PriorityQueueTimer* timer, L limit_value);
    virtual ~RateLimiter();
    /*
    virtual bool IsLimited() = 0;
    virtual void EnqueueTrigger(const ElementType<E, F> > &enqueue_element) = 0;
    virtual void DequeueTrigger(const ElementType<E, F> > &dequeu_element) = 0;
    virtual uint64_t GetAvailableTime() = 0;
    */
   protected:
    L m_limit;
};

template <class E, uint64_t (*F)(const E&)>
Element<E, F>::Element() : m_prio(kNullPriority), m_element() {}
template <class E, uint64_t (*F)(const E&)>
Element<E, F>::~Element() {}

template <class E, uint64_t (*F)(const E&)>
Element<E, F>::Element(Priority prio, const E& e) : m_prio(prio), m_element(e){};
template <class E, uint64_t (*F)(const E&)>
Element<E, F>::Element(Priority prio, const E&& e) : m_prio(prio), m_element(std::forward<E>(e)){};

template <class E, uint64_t (*F)(const E&)>
Element<E, F>::Element(const Element<E, F>& _element) : m_prio(_element.m_prio), m_element(_element.m_element) {}
template <class E, uint64_t (*F)(const E&)>
Element<E, F>::Element(const Element<E, F>&& _element) : m_prio(_element.m_prio), m_element(std::move(_element.m_element)) {}
template <class E, uint64_t (*F)(const E&)>
const Element<E, F>& Element<E, F>::operator=(const Element<E, F>& _element) {
    m_prio = _element.m_prio;
    m_element = _element.m_element;
    return &this;
}
template <class E, uint64_t (*F)(const E&)>
const Element<E, F>& Element<E, F>::operator=(const Element<E, F>&& _element) {
    m_prio = _element.m_prio;
    m_element = std::move(_element.m_element);
    return *this;
}

template <class E, uint64_t (*F)(const E&)>
uint64_t Element<E, F>::GetSize() const {
    return F(m_element);
};

template <class E, uint64_t (*F)(const E&)>
RateLimiterBase<E, F>::RateLimiterBase(PriorityQueueTimer* timer) : m_queue(nullptr), m_timer(timer){};

template <class E, uint64_t (*F)(const E&)>
RateLimiterBase<E, F>::~RateLimiterBase(){};

template <class E, uint64_t (*F)(const E&), class L>
RateLimiter<E, F, L>::~RateLimiter(){};

template <class E, uint64_t (*F)(const E&)>
void RateLimiterBase<E, F>::set_queue(PriorityQueue<E, F>* queue) {
    m_queue = queue;
}

}  // namespace priority_q

#endif /* PRIORITY_Q_RATE_LIMITER_H */