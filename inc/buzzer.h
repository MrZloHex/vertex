#ifndef __BUZZER_H__
#define __BUZZER_H__

#include <stdint.h>

// Initialize the buzzer pin for output.
void buzzer_init(void);

// Generate a tone of a given frequency (in Hz) for a specified duration (in milliseconds).
void buzzer_tone(uint16_t frequency, uint32_t duration_ms);

// Stop any tone (set the buzzer pin to low).
void buzzer_noTone(void);

#endif /* __BUZZER_H__ */

