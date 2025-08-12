#include "buzzer.h"
#include <avr/io.h>

// Make sure F_CPU is defined (e.g., via -DF_CPU=16000000UL in your Makefile)
#ifndef F_CPU
#error "F_CPU must be defined"
#endif

// Define the port and pin for the buzzer (Arduino Nano: digital pin D6 -> PD6).
#define BUZZER_PORT PORTD
#define BUZZER_DDR  DDRD
#define BUZZER_PIN  PD6

// Custom delay function that accepts a runtime variable (microseconds).
// This function uses a busy loop calibrated for the target F_CPU.
// Note: The calibration factor (F_CPU / 4000000UL) is a rough approximation
// and may need adjustment for more precise timing.
static void delay_us_var(uint32_t us) {
    // Calculate iterations per microsecond.
    uint32_t iterations = (F_CPU / 4000000UL);
    while (us--) {
        volatile uint32_t count = iterations;
        while (count--) {
            __asm__ __volatile__("nop");
        }
    }
}

void buzzer_init(void) {
    // Set the buzzer pin as output.
    BUZZER_DDR |= (1 << BUZZER_PIN);
}

void buzzer_tone(uint16_t frequency, uint32_t duration_ms) {
    // Calculate half the period in microseconds.
    // For a square wave, the output toggles every half period.
    uint32_t half_period_us = 1000000UL / (frequency * 2UL);
    uint32_t duration_us = duration_ms * 1000UL;
    uint32_t elapsed_us = 0;

    while (elapsed_us < duration_us) {
        // Turn the buzzer on.
        BUZZER_PORT |= (1 << BUZZER_PIN);
        delay_us_var(half_period_us);
        
        // Turn the buzzer off.
        BUZZER_PORT &= ~(1 << BUZZER_PIN);
        delay_us_var(half_period_us);
        
        elapsed_us += (2 * half_period_us);
    }
}

void buzzer_noTone(void) {
    // Ensure the buzzer is turned off.
    BUZZER_PORT &= ~(1 << BUZZER_PIN);
}

