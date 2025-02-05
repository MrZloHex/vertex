/* =============================================================================
 *                          ░▒▓█ _VERTEX_ █▓▒░
 *
 *   File       : usart.h
 *   Author     : MrZloHex
 *   Date       : 2025-02-05
 *
 *   Description:
 *
 *
 * =============================================================================
 */

#ifndef __USART_H__
#define __USART_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>


#ifndef USART_RX_BUFFER_SIZE
#define USART_RX_BUFFER_SIZE 128
#endif

#ifndef USART_PROCESS_BUFFER_SIZE
#define USART_PROCESS_BUFFER_SIZE 128
#endif


#define USART_ERROR_FRAME   (1 << 0)
#define USART_ERROR_OVERRUN (1 << 1)
#define USART_ERROR_PARITY  (1 << 2)

typedef void (*usart_callback_t)(const uint8_t *message, uint8_t length);

typedef void (*usart_error_callback_t)(uint8_t error_flags);

typedef struct {
    volatile uint8_t rx_buffer[USART_RX_BUFFER_SIZE];
    volatile uint8_t process_buffer[USART_PROCESS_BUFFER_SIZE];
    volatile uint8_t rx_head;
    volatile uint8_t rx_tail;
    volatile uint8_t process_head;
    volatile uint8_t process_tail;
    volatile uint8_t term_symbol;

    volatile uint16_t err_frame;
    volatile uint16_t err_overrun;
    volatile uint16_t err_parity;

    volatile usart_callback_t msg_callback;
    volatile usart_error_callback_t error_callback;
} USART_Buffer_t;


void
usart_init(USART_Buffer_t *usartData, uint16_t baudrate);

void 
usart_set_callback(USART_Buffer_t *usartData, usart_callback_t callback);

void
usart_set_error_callback(USART_Buffer_t *usartData, usart_error_callback_t callback);

void
usart_send(uint8_t data);

void 
usart_send_string(const char *str);

bool
usart_read_byte(USART_Buffer_t *usartData, uint8_t *data);

uint8_t
usart_read_message
(
    USART_Buffer_t *usartData,
    uint8_t *buffer,
    uint8_t max_length
);

void
usart_get_errors
(
    USART_Buffer_t *usartData,
    uint16_t *frame,
    uint16_t *overrun,
    uint16_t *parity
);

void
usart_clear_errors(USART_Buffer_t *usartData);

#endif /* __USART_H__ */

