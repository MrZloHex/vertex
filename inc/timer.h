/**
 ******************************************************************************
 * @file    timer.h
 * @author  Zlobin Aleksey
 * @date    2022-08-13
 * @brief   Simple software timer utility for STM32 platforms.
 *          This file contains the declarations for a basic timer structure
 *          and its associated functions.
 ******************************************************************************
 * @attention
 *
 * The Timer utility is meant to facilitate non-blocking delays by comparing
 * current system ticks (from HAL_GetTick) with a stored target time.
 * This file can be included anywhere timing functionality is required.
 *
 ******************************************************************************
 * Do this:
 *    #define TIMER_IMPL
 * before you include this file in *one* C or file to create the implementation.
 *
 * // i.e. it should look like this:
 * #include ...
 * #include ...
 * #include ...
 * #define TIMER_IMPL
 * #include "timer.h"
 *
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include "stdint.h"
#include "stdbool.h"

/**
 * @brief  Timer structure that holds configuration and state.
 *
 * - timeout:  Duration of the timer in milliseconds.
 * - target:   The system tick value at which the timer will expire.
 * - start:    Flag indicating whether the timer is currently active.
 * - restart:  Flag indicating whether the timer will automatically
 *             restart after it times out.
 */
typedef struct Timer_S
{
    uint32_t timeout;
    uint32_t target;
    bool     start;
    bool     restart;
} Timer;

/**
 * @brief  Configure a Timer object with a specified timeout and restart behavior.
 * @param  t         Pointer to a Timer structure
 * @param  timeout   Desired timeout period in milliseconds
 * @param  restart   If true, the timer will restart each time it expires
 * @retval None
 */
void
timer_set(Timer *t, uint32_t timeout, bool restart);

/**
 * @brief  Start the timer, setting the target tick for expiration.
 * @param  t   Pointer to a Timer structure
 * @retval None
 */
void
timer_start(Timer *t);

/**
 * @brief  Check if the timer has timed out.
 * @param  t   Pointer to a Timer structure
 * @retval true  Timer has timed out
 * @retval false Timer has not yet timed out or is not active
 */
bool
timer_timeout(Timer *t);

/**
 * @brief  Stop the timer and disable any time checks.
 * @param  t   Pointer to a Timer structure
 * @retval None
 */
void
timer_stop(Timer *t);

#endif /* __TIMER_H__ */

#ifdef TIMER_IMPL
#ifndef TIMER_GET_TICK
# error "Please define function to get tick"
#endif

uint32_t TIMER_GET_TICK(void);

void
timer_set(Timer *t, uint32_t timeout, bool restart)
{
    t->restart = restart;
    t->timeout = timeout;
}

void
timer_start(Timer *t)
{
    if (!t->start)
    {
        t->start = true;
        t->target = TIMER_GET_TICK() + t->timeout;
    }
}

bool
timer_timeout(Timer *t)
{
    if (!(t->start && t->target <= TIMER_GET_TICK()))
    { return false; }

    if (t->restart)
    { t->target += t->timeout; }
    else
    { t->start = false; }

    return true;
}

void
timer_stop(Timer *t)
{
    t->start = false;
}

#endif

