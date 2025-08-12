#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t g_ms = 0;

void
timer_init(void)
{
    /* Timer0 CTC at 1kHz: F_CPU=16MHz, presc=64 => 250kHz; OCR0A=249 => 1kHz */
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS01) | _BV(CS00); /* prescaler 64 */
    OCR0A  = 249;
    TIMSK0 = _BV(OCIE0A);
}

ISR(TIMER0_COMPA_vect)
{
    g_ms++;
}

uint32_t
millis(void)
{
    uint32_t ms;
    uint8_t s = SREG;
    cli();
    ms  = g_ms;
    SREG = s;
    return ms;
}

int
elapsed(uint32_t since_ms, uint32_t period_ms)
{
    return (uint32_t)(millis() - since_ms) >= period_ms ? 1 : 0;
}
