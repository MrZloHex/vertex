#ifndef __LED_H__
#define __LED_H__

#include <stdint.h>

typedef enum
{
    LED_MODE_SOLID = 1,
    LED_MODE_FADE  = 2,
    LED_MODE_BLINK = 3
} led_mode_t;

typedef struct
{
    led_mode_t    mode;         /* current mode          */
    uint8_t       brightness;   /* 0..255 for SOLID      */
    uint8_t       actual_bright;   /* 0..255 for SOLID      */
    uint8_t       state;        /* on or off             */
} led_state_t;

void
effects_init(void);

void
effects_set_mode(led_mode_t mode);

void
effects_set_state(uint8_t state);

void
effects_set_brightness(uint8_t b);

led_state_t
effects_get(void);

void
effects_tick_1ms(void); /* call from main loop each ms tick */

#endif /* __LED_H__ */
