/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201104
 * brief:    util header file
 */

#ifndef PRIORITY_Q_UTIL_H
#define PRIORITY_Q_UTIL_H
#include <cassert>
#include <cerrno>
#include <iostream>

#ifdef __GNUC__
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#ifndef NDBUG
#define KASSERT(x)                                            \
    do {                                                      \
        if (!(x)) {                                           \
            std::cout << "asset failed: " << #x << std::endl; \
            abort();                                          \
        }                                                     \
    } while (0)
#else
#define #define KASSERT(x)
#endif

#endif /* PRIORITY_Q_UTIL_H */