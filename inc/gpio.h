#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>

void
gpio_init(void);

void
lamp_set(uint8_t on);

uint8_t
lamp_get(void);

void
buzzer_set(uint8_t on);

void
led_pwm_set(uint8_t duty);

#endif /* __GPIO_H__ */
