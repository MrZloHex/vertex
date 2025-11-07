#include "led.h"
#include "gpio.h"
#include "timer.h"

static led_state_t s_led = { .mode = LED_MODE_OFF, .brightness = 0 };
static int16_t     s_dir = 1; /* for FADE ramp */
static bool        s_blink = true;
static Timer       s_led_tim;

void
effects_init(void)
{
    s_led.mode       = LED_MODE_OFF;
    s_led.brightness = 0;
    led_pwm_set(0);
    timer_set(&s_led_tim, 100, true);
    timer_start(&s_led_tim);
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
    if (!timer_timeout(&s_led_tim))
    { return; }

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
            led_pwm_set(s_blink * s_led.brightness);
            s_blink = !s_blink; 
        } break;
    }
}
