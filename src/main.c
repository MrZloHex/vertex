#if 0
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN     PB5

#include "trace.h"

#include "usart.h"
USART_Buffer_t usart_data;

#define SERIAL_SEND_IMPL usart_send_data
#include "serial.h"

#include "protocol.h"

void message_callback(const uint8_t *message, uint8_t length) {
    char *raw_msg = (char *)malloc(length);
    strcpy(raw_msg, (const char *)message);
    raw_msg[length-1] = 0;

    TRACE_DEBUG("RECV `%s`", raw_msg);

    MLP_Msg mlp_msg = mlp_parse_msg(raw_msg);

    TRACE_INFO
    (
        "RECV\t%s %s `%s` `%s` `%s` `%s`",
        MLP_MsgType_STR[mlp_msg.type],
        MLP_MsgAction_STR[mlp_msg.action],
        mlp_msg.params[0],
        mlp_msg.params[1],
        mlp_msg.params[2],
        mlp_msg.params[3]
    );

    free(raw_msg);
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

#include "tick.h"

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

    tracer_init(TRC_DEBUG, TP_ALL);
    TRACE_INFO("HUY HUYHUYUP");


    sei();

    Timer t = { 0 };
    timer_set(&t, 500, true);
    timer_start(&t);


    while (1)
    {
        if (timer_timeout(&t))
        {
            PORTB ^= (1 << LED_PIN);
        }
    }

    return 0;
}



#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    // Configure PD6 (Arduino digital pin 6) as an output.
    DDRD |= (1 << PD6);

    while (1) {
        // Turn the buzzer on by setting PD6 high.
        PORTD |= (1 << PD6);
        _delay_ms(500);  // Wait 500 milliseconds

        // Turn the buzzer off by setting PD6 low.
        PORTD &= ~(1 << PD6);
        _delay_ms(500);  // Wait 500 milliseconds
    }

    return 0;
}


#endif 
    
#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define MAX_DUTY 500  // ICR1 value, defines the maximum duty cycle

int main(void) {
    // Set D9 (PB1) as output.
    DDRB |= (1 << PB1);

    // Configure Timer1 in Fast PWM mode (Mode 14) using ICR1 as TOP.
    // COM1A1: Non-inverting mode on OC1A (D9)
    // WGM13, WGM12, WGM11: Set Fast PWM mode with ICR1 as TOP.
    // CS10: No prescaling (adjust if needed).
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

    // Set TOP value to define the PWM frequency.
    ICR1 = MAX_DUTY;

    // Start with the LED off.
    OCR1A = 0;

    // Variables to control the duty cycle.
    uint16_t duty = 0;
    int8_t step = 1;  // Increment (or decrement) for duty cycle

    while (1) {
        // Update the duty cycle.
        OCR1A = duty;
        // Delay for a visible fade effect.
        _delay_ms(10);

        // Increment or decrement the duty cycle.
        duty += step;

        // Reverse direction when reaching the limits.
        if (duty == 0 || duty >= MAX_DUTY) {
            step = -step;
        }
    }

    return 0;
}


