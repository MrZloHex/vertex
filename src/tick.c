#include "tick.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t g_millis = 0;

ISR(TIMER0_COMPA_vect)
{
    g_millis++;
}

void
timer0_init(void)
{
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = (F_CPU / (64UL * 1000UL)) - 1;
    TIMSK0 = (1 << OCIE0A);
    sei();
}

uint32_t
avr_get_tick(void)
{
    uint32_t millis;
    uint8_t sreg = SREG;
    cli();
    millis = g_millis;
    SREG = sreg;
    return millis;
}

