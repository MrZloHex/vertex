#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN     PB5

#include "usart.h"
USART_Buffer_t usart_data;

void message_callback(const uint8_t *message, uint8_t length) {
    // Echo the message back with a prefix.
    usart_send_string("Got: `");
    for (uint8_t i = 0; i < length; i++) {
        usart_send(message[i]);
    }
    usart_send_string("`\n");
}

void error_callback(uint8_t error_flags) {
    usart_send_string("USART Error: ");
    if (error_flags & USART_ERROR_FRAME) {
        usart_send_string("Frame ");
    }
    if (error_flags & USART_ERROR_OVERRUN) {
        usart_send_string("Overrun ");
    }
    if (error_flags & USART_ERROR_PARITY) {
        usart_send_string("Parity ");
    }
    usart_send_string("\n");
}

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint32_t g_millis = 0;

ISR(TIMER0_COMPA_vect)
{
    g_millis++;
}

void timer0_init(void)
{
    // Configure Timer0 in CTC mode with a compare value that gives a 1 ms period.
    // Assuming an 8 MHz clock with a prescaler of 64:
    // OCR0A = (F_CPU / (prescaler * 1000)) - 1
    TCCR0A = (1 << WGM01);                   // CTC mode
    TCCR0B = (1 << CS01) | (1 << CS00);        // Prescaler 64
    OCR0A = (F_CPU / (64UL * 1000UL)) - 1; // Compare match value for 1 ms
    TIMSK0 = (1 << OCIE0A);                    // Enable compare match interrupt
}

uint32_t avr_get_tick(void)
{
    uint32_t millis;
    // To prevent race conditions, disable interrupts temporarily.
    uint8_t sreg = SREG;
    cli();
    millis = g_millis;
    SREG = sreg;
    return millis;
}



#define TIMER_GET_TICK avr_get_tick
#define TIMER_IMPL
#include "timer.h"

int
main(void)
{
    DDRB |= (1 << LED_PIN);

    usart_init(&usart_data, 9600);
    usart_data.term_symbol = '\n';

    usart_set_callback(&usart_data, message_callback);
    usart_set_error_callback(&usart_data, error_callback);

    timer0_init();
    sei();

    usart_send_string("Enter a message:\n");


    Timer t = { 0 };
    timer_set(&t, 1000, true);
    timer_start(&t);

    bool flag = true;

    while (1)
    {
        if (timer_timeout(&t))
        {
            if (flag)
            {
                PORTB |= (1 << LED_PIN);
                flag = false;
            }
            else
            {
                PORTB &= ~(1 << LED_PIN);
                flag = true;
            }
        }
    }

    return 0;
}
