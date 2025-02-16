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
    TRACE_INFO("BOOTING UP");


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
