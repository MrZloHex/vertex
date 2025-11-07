#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"
#include "gpio.h"
#include "protocol.h"
#include "led.h"
#include "alarm.h"
#define TIMER_IMPL
#include "timer.h"


int
main(void)
{
    cli();

    gpio_init();
    timer_init();
    alarm_init();
    uart_init(BAUD);

    sei();

    proto_init();

    for (;;)
    {
        proto_poll();
        effects_tick_1ms();
        alarm_loop();
    }
}
