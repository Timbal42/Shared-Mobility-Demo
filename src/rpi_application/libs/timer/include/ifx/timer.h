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
 * \file ifx/timer.h
 * \brief Generic API for joinable timers
 */
#ifndef _IFX_TIMER_H_
#define _IFX_TIMER_H_

#include <stdbool.h>
#include <stdint.h>
#include "ifx/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error code module identifer
 */
#define LIBTIMER 0x02

/**@struct Timer
 * \brief Joinable Timer
 */
typedef struct
{
    /**
     * \brief Private start of timer as \c void* to be as generic as possible
     *
     * \details Set by \ref timer_set(Timer*, uint64_t), do **NOT** set manually
     */
    void *_start;

    /**
     * \brief Private duriation in [us]
     *
     * \details Set by \ref timer_set(Timer*, uint64_t), do **NOT** set manually
     */
    uint64_t _duration; /**< Duration in [us] */
} Timer;

/**
 * \brief Return code for successful calls to \ref timer_set(Timer*, uint64_t)
 */
#define TIMER_SET_SUCCESS SUCCESS

/**
 * \brief IFX error encoding function identifier for \ref timer_set(Timer*, uint64_t)
 */
#define TIMER_SET 0x01

/**
 * \brief Sets \ref Timer for given amount of [us]
 *
 * \param timer Timer object to be set
 * \param us Microseconds for timer
 * \return int \c TIMER_SET_SUCCESS if successful, any other value in case of error
 */
int timer_set(Timer *timer, uint64_t us);

/**
 * \brief Checks if \ref Timer has elapsed
 *
 * \details Per definition timers that have not previously been set are considered elapsed
 *
 * \param timer Timer object to be checked
 * \return bool \c true if timer has elapsed
 */
bool timer_has_elapsed(Timer *timer);

/**
 * \brief Return code for successful calls to \ref timer_join(Timer*)
 */
#define TIMER_JOIN_SUCCESS SUCCESS

/**
 * \brief IFX error encoding function identifier for \ref timer_join(Timer*)
 */
#define TIMER_JOIN 0x02

/**
 * \brief Error reason if timer has not been set before calling \ref timer_join(Timer*)
 */
#define TIMER_NOT_SET 0x01

/**
 * \brief Waits for \ref Timer to finish
 *
 * \param timer Timer to be joined
 * \return int \c TIMER_JOIN_SUCCESS if successful, any other value in case of error
 */
int timer_join(Timer *timer);

/**
 * \brief Frees memory associated with \ref Timer object (but not object itself)
 *
 * \details Timer objects may contain dynamically allocated data that might need special functionality to be freed.
 *          Users would need to know the concrete type based on the linked implementation. This would negate all benefits of having a generic interface.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param timer Timer object whose data shall be freed
 */
void timer_destroy(Timer *timer);

#ifdef __cplusplus
}
#endif

#endif // _IFX_TIMER_H_
