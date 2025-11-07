/**
 ******************************************************************************
 * @file    timer.h
 * @author  Zlobin Aleksey
 * @date    2024-11-07
 * @brief   Simple software timer utility for AVR platforms.
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
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

#if   F_CPU == 16000000UL
#   define TIMER0_OCR_FOR_1MS   (uint8_t)(16000000UL / (64UL * 1000UL) - 1)  /* 249 */
#elif F_CPU == 8000000UL
#   define TIMER0_OCR_FOR_1MS   (uint8_t)(8000000UL  / (64UL * 1000UL) - 1)  /* 124 */
#elif F_CPU == 20000000UL
#   define TIMER0_OCR_FOR_1MS   (uint8_t)(20000000UL / (64UL * 1000UL) - 1)  /* 312 -> clipped by 8-bit timer, so use prescaler 128 on some AVRs if available */
/* If your MCU lacks prescaler/CTC combination for 20 MHz, either change F_CPU or
   define TIMER_AVR_EXTERNAL_MILLIS and supply your own tick. */
#   warning "At 20 MHz with Timer0 8-bit CTC + prescaler 64, OCR0A exceeds 255. Consider external millis or adjust prescaler if your AVR supports different Timer0 prescalers."
#else
#   warning "Unsupported F_CPU for built-in 1ms tick. Define TIMER_AVR_EXTERNAL_MILLIS and provide g_millis."
#endif

void
timer_init(void);

uint32_t
timer_now(void);

bool
timer_reached(uint32_t now, uint32_t target);

typedef struct
{
    bool      start;
    bool      repeat;
    uint32_t  timeout;
    uint32_t  target;
} Timer;

void
timer_set(Timer *t, uint32_t timeout, bool repeat);

void
timer_start(Timer *t);

void
timer_stop(Timer *t);

bool
timer_timeout(Timer *t);

#endif /* __TIMER_H__ */

#ifdef TIMER_IMPL

#ifndef TIMER_AVR_EXTERNAL_MILLIS
volatile uint32_t g_millis = 0;

void
timer_init(void)
{
    /* Stop Timer0 */
    TCCR0A = 0;
    TCCR0B = 0;

#ifdef TIMER0_OCR_FOR_1MS
    /* CTC mode: WGM01 = 1 */
    TCCR0A |= (1 << WGM01);
    /* Set OCR0A for 1 ms */
    OCR0A = TIMER0_OCR_FOR_1MS;
    /* Clear pending flag */
    TIFR0  |= (1 << OCF0A);
    /* Enable Compare Match A interrupt */
    TIMSK0 |= (1 << OCIE0A);
    /* Start with prescaler 64: CS01=1, CS00=1 */
    TCCR0B |= (1 << CS01) | (1 << CS00);
#endif
}

ISR(TIMER0_COMPA_vect)
{ g_millis++; }

#else  /* TIMER_AVR_EXTERNAL_MILLIS */

extern volatile uint32_t g_millis;

void
timer_init(void)
{
    /* No-op: external project must increment g_millis every 1 ms */
}

#endif /* TIMER_AVR_EXTERNAL_MILLIS */

uint32_t
timer_now(void)
{
    uint32_t ms;
    uint8_t sreg = SREG;
    cli();
    ms = g_millis;
    SREG = sreg;

    return ms;
}

bool
timer_reached(uint32_t now, uint32_t target)
{
    return (int32_t)(now - target) >= 0;
}

void
timer_set(Timer *t, uint32_t timeout, bool repeat)
{
    t->timeout = timeout;
    t->repeat  = repeat;
    t->start   = false;
}

void
timer_start(Timer *t)
{
    t->target = timer_now() + t->timeout;
    t->start  = true;
}

void
timer_stop(Timer *t)
{
    t->start = false;
}

bool
timer_timeout(Timer *t)
{
    if (!t->start)
    { return false; }

    uint32_t now = timer_now();

    if (!timer_reached(now, t->target))
    { return false; }

    if (t->repeat)
    { t->target += t->timeout; }
    else
    { t->start = false; }

    return true;
}

#endif /* TIMER_IMPL */
