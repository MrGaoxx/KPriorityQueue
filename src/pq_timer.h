/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    a timer interface
 */

#ifndef PRIORITY_Q_TIMER_H
#define PRIORITY_Q_TIMER_H
#include <cstdint>

namespace priority_q {

class PriorityQueueTimer {
   public:
    PriorityQueueTimer();
    virtual ~PriorityQueueTimer();
    virtual uint64_t GetNowInUs() = 0;
};

inline PriorityQueueTimer::PriorityQueueTimer() {}
inline PriorityQueueTimer::~PriorityQueueTimer() {}

}  // namespace priority_q

#endif /* PRIORITY_Q_TIMER_H */