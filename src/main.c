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

int
main(void)
{
    DDRB |= (1 << LED_PIN);

    usart_init(&usart_data, 9600);
    usart_data.term_symbol = '\n';

    usart_set_callback(&usart_data, message_callback);
    usart_set_error_callback(&usart_data, error_callback);

    sei();

    usart_send_string("Enter a message:\n");


    while (1)
    {

        PORTB |= (1 << LED_PIN);
        _delay_ms(1000);
        PORTB &= ~(1 << LED_PIN);
        _delay_ms(500);
    }

    return 0;
}
