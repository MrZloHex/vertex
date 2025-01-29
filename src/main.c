#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define LED_PIN     PB5

#include "usart.h"

void
read_str(char *buffer, size_t size, char term)
{
    size_t i = 0;
    while (i < size)
    {
        char c = usart_recv_char();
        if (c == term)
        {
            buffer[i] = 0;
            break;
        }

        buffer[i++] = c;
    }
}

int
main(void)
{
    DDRB |= (1 << LED_PIN);
    usart_init(9600);
    usart_send_str("HELLO FROM C\n");
    char buf[100];
    read_str(buf, sizeof(buf), '\n');
    usart_send_str("YOU SEND `");
    usart_send_str(buf);
    usart_send_str("`\n");

    while (1)
    {
        PORTB |= (1 << LED_PIN);
        _delay_ms(1000);
        PORTB &= ~(1 << LED_PIN);
        _delay_ms(500);
    }

    return 0;
}
