//
// Created by Colin on 2021/3/31.
//

#ifndef STRATEGY_UTILS_H
#define STRATEGY_UTILS_H

#include <cstdlib>

using namespace std;

typedef pair<int, int> intPair;

/**
 * @param x
 * @param y
 * @return random int in [x, y)
 */
inline int randint(int x = 0, int y = 0x7fffffff)
{
    return x + rand() % (y - x);
}

void record_time(timespec& _time)
{
    clock_gettime(CLOCK_MONOTONIC, &_time);
}

/**
 * compute delta time
 * @return delta time in ms
 */
double delta_time(const timespec& start, const timespec& end)
{
    return (end.tv_sec - start.tv_sec) * 1e3 +
            (end.tv_nsec - start.tv_nsec) * 1e-6;
}

/**
 * @return delta time from a certain moment to now
 */
double delta_time_from(const timespec& start)
{
    timespec now;
    record_time(now);
    return delta_time(start, now);
}

#endif //STRATEGY_UTILS_H
