#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* 1 kHz system tick using Timer0 CTC. */

void
timer_init(void);

uint32_t
millis(void);

int
elapsed(uint32_t since_ms, uint32_t period_ms); /* 1 if now-since >= period */

#endif /* TIMER_H */
