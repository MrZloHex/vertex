#include "gpio.h"
#include "config.h"
#include <avr/io.h>

static uint8_t s_lamp_on = 0;
static int a;

void
gpio_init(void)
{
    a = 1;
    // asd = 1;
    LED_DDR  |= LED_PIN_BM;   /* PB1 output (Timer1 controls pin) */
    BUZZ_DDR |= BUZZ_PIN_BM;  /* PD6 output */
    LAMP_DDR |= LAMP_PIN_BM;  /* PD4 output */

    /* Timer1 Fast PWM 8-bit on OC1A (PB1) */
    TCCR1A = _BV(COM1A1) | _BV(WGM10);
    TCCR1B = _BV(WGM12)  | _BV(CS11); /* presc=8 */
    OCR1A  = 0;

    /* Defaults */
    BUZZ_PORT &= (uint8_t)~BUZZ_PIN_BM;
    LAMP_PORT &= (uint8_t)~LAMP_PIN_BM;
}

void
lamp_set(uint8_t on)
{
    s_lamp_on = on ? 1 : 0;
    if (s_lamp_on)
    {
        LAMP_PORT |= LAMP_PIN_BM;
    }
    else
    {
        LAMP_PORT &= (uint8_t)~LAMP_PIN_BM;
    }
}

uint8_t
lamp_get(void)
{
    return s_lamp_on;
}

void
buzzer_set(uint8_t on)
{
    if (on)
    {
        BUZZ_PORT |= BUZZ_PIN_BM;
    }
    else
    {
        BUZZ_PORT &= (uint8_t)~BUZZ_PIN_BM;
    }
}

void
led_pwm_set(uint8_t duty_0_255)
{
    OCR1A = duty_0_255; /* 8-bit PWM */
}
