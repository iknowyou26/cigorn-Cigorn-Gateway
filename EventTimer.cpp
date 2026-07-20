/*
 * File:   EventTimer.cpp
 * Author: Ryan Le
 * Date: July 16, 2026
 *
 * Cross-platform implementation using std::chrono
 */

#include "EventTimer.h"

#include <chrono>

namespace
{
double CurrentTimeSeconds()
{
    const auto now =
        std::chrono::steady_clock::now().time_since_epoch();

    return std::chrono::duration<double>(now).count();
}
}

EventTimer::EventTimer()
{
    starttime = CurrentTimeSeconds();
    nowtime = starttime;
}

EventTimer::EventTimer(const EventTimer& orig)
{
    starttime = orig.starttime;
    nowtime = orig.nowtime;
}

EventTimer::~EventTimer()
{
}

void EventTimer::start()
{
    starttime = CurrentTimeSeconds();
    nowtime = starttime;
}

double EventTimer::timeinterval()
{
    nowtime = CurrentTimeSeconds();
    return nowtime - starttime;
}