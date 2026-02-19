#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 57600
#endif

#ifndef FW_VERSION
#define FW_VERSION "2.1.1"
#endif

/* Pin mapping (Arduino Nano):
 * D9  -> PB1 (OC1A)  LED strip via MOSFET
 * D6  -> PD6         Active buzzer (digital)
 * D4  -> PD4         Relay (desk lamp)
 */
#define LED_PORT       PORTB
#define LED_DDR        DDRB
#define LED_PIN_BM     _BV(PB1)     /* OC1A */

#define BUZZ_PORT      PORTD
#define BUZZ_DDR       DDRD
#define BUZZ_PIN_BM    _BV(PD6)

#define LAMP_PORT      PORTD
#define LAMP_DDR       DDRD
#define LAMP_PIN_BM    _BV(PD4)

/* Registration / retry */
#define REG_ACK_TIMEOUT_MS   2000UL
#define REG_RETRY_PERIOD_MS  5000UL

/* Parser */
#define RX_LINE_MAX          256

#endif /* __CONFIG_H__ */
