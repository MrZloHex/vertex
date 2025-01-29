#include "usart.h"
#include <avr/io.h>

void
usart_init(const uint32_t baudrate)
{
    uint16_t ubrr = (F_CPU / 16 / baudrate) - 1;

    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;

    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void
usart_send_char(const char c)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void
usart_send_str(const char *str)
{
    while (*str)
    {
        usart_send_char(*str++);
    }
}

char
usart_recv_char()
{
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

