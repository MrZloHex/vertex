#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"
#include "timer.h"
#include "gpio.h"
#include "protocol.h"
#include "effects.h"


int
main(void)
{
    cli();

    gpio_init();
    timer_init();
    uart_init(BAUD);

    sei();

    proto_init();

    for (;;)
    {
        proto_poll();
        effects_tick_1ms();
    }
}
