/*
 * MIT License
 *
 * Copyright (c) 2020 Infineon Technologies AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * \file chronotimer.cpp
 * \brief C timer implementation based on C++11 chrono
 */
#include <chrono>
#include <thread>
#include "ifx/timer.h"

using namespace std;
using namespace chrono;

/**
 * \brief Gets the \ref time_point at which the \ref Timer will elapse / has elapsed
 *
 * \param Timer Timer to get end point for
 * \return time_point<system_clock> at which the timer has elpased / will elapse
 */
static time_point<system_clock> get_end(Timer *timer)
{
    // Safety check to ensure that timer has been set
    if (timer->_start == NULL)
    {
        return system_clock::now();
    }

    return (*((time_point<system_clock> *)timer->_start)) + duration<int, micro>(timer->_duration);
}

/**
 * \brief Sets \ref Timer for given amount of [us]
 *
 * \param timer Timer object to be set
 * \param us Microseconds for timer
 * \return int \c TIMER_SET_SUCCESS if successful, any other value in case of error
 */
int timer_set(Timer *timer, uint64_t us)
{
    // Validate parameters
    if (timer == NULL)
    {
        return IFX_ERROR(LIBTIMER, TIMER_SET, ILLEGAL_ARGUMENT);
    }

    // Actually set parameters
    timer->_start = new time_point<system_clock>(system_clock::now());
    timer->_duration = us;
    return TIMER_SET_SUCCESS;
}

/**
 * \brief Checks if \ref Timer has elapsed
 *
 * \details Per definition timers that have not previously been set are considered elapsed
 *
 * \param timer Timer object to be checked
 * \return bool \c true if timer has elapsed
 */
bool timer_has_elapsed(Timer *timer)
{
    // Assure that timer has been set
    if (timer->_start == NULL)
    {
        return true;
    }

    auto end_time = get_end(timer);
    return end_time < system_clock::now();
}

/**
 * \brief Waits for \ref Timer to finish
 *
 * \param timer Timer to be joined
 * \return int \c TIMER_JOIN_SUCCESS if successful, any other value in case of error
 */
int timer_join(Timer *timer)
{
    // Assure that timer has been set
    if (timer->_start == NULL)
    {
        return IFX_ERROR(LIBTIMER, TIMER_JOIN, TIMER_NOT_SET);
    }

    // Sleep until the end
    auto end_time = get_end(timer);
    this_thread::sleep_until(end_time);
    return TIMER_JOIN_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref Timer object (but not object itself)
 *
 * \details Timer objects may contain dynamically allocated data that might need special functionality to be freed.
 *          Users would need to know the concrete type based on the linked implementation. This would negate all benefits of having a generic interface.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param timer Timer object whose data shall be freed
 */
void timer_destroy(Timer *timer)
{
    if (timer->_start != NULL)
    {
        delete ((time_point<system_clock> *)timer->_start);
    }
    timer->_start = NULL;
}
