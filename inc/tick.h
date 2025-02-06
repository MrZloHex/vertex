#ifndef __TICK_H__
#define __TICK_H__ 

#include <stdint.h>

void
timer0_init(void);

uint32_t
avr_get_tick(void);

#endif /* __TICK_H__ */

