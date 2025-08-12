#include "effects.h"
#include "gpio.h"
#include "timer.h"

static led_state_t s_led = { .mode = LED_MODE_OFF, .brightness = 0 };
static uint32_t    s_last_ms = 0;
static int16_t     s_dir = 1; /* for FADE ramp */

void
effects_init(void)
{
    s_led.mode       = LED_MODE_OFF;
    s_led.brightness = 0;
    led_pwm_set(0);
}

void
effects_set_mode(led_mode_t mode)
{
    s_led.mode = mode;
    if (mode == LED_MODE_OFF)
    {
        led_pwm_set(0);
    }
}

void
effects_set_brightness(uint8_t b)
{
    s_led.brightness = b;
    if (s_led.mode == LED_MODE_SOLID)
    {
        led_pwm_set(b);
    }
}

led_state_t
effects_get(void)
{
    return s_led;
}

void
effects_tick_1ms(void)
{
    uint32_t now = millis();
    if (!elapsed(s_last_ms, 10)) { return; } /* 100 Hz update */
    s_last_ms = now;

    switch (s_led.mode)
    {
        case LED_MODE_OFF:
            /* nothing */
            break;
        case LED_MODE_SOLID:
            /* steady brightness already applied */
            break;
        case LED_MODE_FADE:
        {
            int16_t v = (int16_t)s_led.brightness + s_dir * 5;
            if (v >= 255) { v = 255; s_dir = -1; }
            if (v <= 0)   { v = 0;   s_dir =  1; }
            s_led.brightness = (uint8_t)v;
            led_pwm_set(s_led.brightness);
        } break;
        case LED_MODE_BLINK:
        {
            /* 1 Hz blink, 50% duty */
            if ((now / 500UL) & 1UL)
            {
                led_pwm_set(0);
            }
            else
            {
                led_pwm_set(s_led.brightness);
            }
        } break;
    }
}
