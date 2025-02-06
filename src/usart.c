/* =============================================================================
 *                          ░▒▓█ _VERTEX_ █▓▒░
 *
 *   File       : usart.c
 *   Author     : MrZloHex
 *   Date       : 2025-02-05
 *
 *   Description:
 *
 *
 * =============================================================================
 */

#include "usart.h"

static USART_Buffer_t *usart_data_ptr = 0;

static inline bool
is_buffer_empty(volatile uint8_t head, volatile uint8_t tail)
{ return (head == tail); }

static inline bool
is_buffer_full(uint8_t head, uint8_t tail, uint8_t buffer_size)
{ return (((head + 1) % buffer_size) == tail); }

void
usart_init(USART_Buffer_t *usartData, uint16_t baud)
{
    usart_data_ptr = usartData;

    usartData->rx_head      = 0;
    usartData->rx_tail      = 0;
    usartData->process_head = 0;
    usartData->process_tail = 0;
    usartData->term_symbol  = '\n';

    usartData->err_frame   = 0;
    usartData->err_overrun = 0;
    usartData->err_parity  = 0;

    usartData->msg_callback   = 0;
    usartData->error_callback = 0;

    uint16_t ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void
usart_set_callback(USART_Buffer_t *usartData, usart_callback_t callback)
{ usartData->msg_callback = callback; }

void
usart_set_error_callback(USART_Buffer_t *usartData, usart_error_callback_t errorCallback)
{ usartData->error_callback = errorCallback; }

void
usart_send(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = data;
}

uint8_t
usart_send_data(uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    { usart_send(data[i]); }

    return 0; 
}

void
usart_send_string(const char *str)
{
    while (*str)
    { usart_send(*str++); }
}

static void
process_rx_data(USART_Buffer_t *usartData)
{
    while (usartData->rx_tail != usartData->rx_head)
    {
        uint8_t byte = usartData->rx_buffer[usartData->rx_tail];
        usartData->rx_tail = (usartData->rx_tail + 1) % USART_RX_BUFFER_SIZE;

        if (!is_buffer_full(usartData->process_head, usartData->process_tail, USART_PROCESS_BUFFER_SIZE))
        {
            usartData->process_buffer[usartData->process_head] = byte;
            usartData->process_head = (usartData->process_head + 1) % USART_PROCESS_BUFFER_SIZE;
        }

        if (byte == usartData->term_symbol)
        {
            if (usartData->msg_callback)
            {
                uint8_t message[USART_PROCESS_BUFFER_SIZE];
                uint8_t i = 0;
                while (!is_buffer_empty(usartData->process_head, usartData->process_tail))
                {
                    message[i] = usartData->process_buffer[usartData->process_tail];
                    usartData->process_tail = (usartData->process_tail + 1) % USART_PROCESS_BUFFER_SIZE;
                    i += 1;

                    if (message[i - 1] == usartData->term_symbol || i >= (USART_PROCESS_BUFFER_SIZE - 1))
                    { break; }
                }
                usartData->msg_callback(message, i);
            }
        }
    }
}

bool
usart_read_byte(USART_Buffer_t *usartData, uint8_t *data)
{
    if (is_buffer_empty(usartData->process_head, usartData->process_tail))
    { return false; }

    *data = usartData->process_buffer[usartData->process_tail];
    usartData->process_tail = (usartData->process_tail + 1) % USART_PROCESS_BUFFER_SIZE;
    return true;
}

uint8_t
usart_read_message(USART_Buffer_t *usartData, uint8_t *buffer, uint8_t max_length)
{
    uint8_t count = 0;
    uint8_t tail = usartData->process_tail;
    while (tail != usartData->process_head && count < max_length)
    {
        buffer[count] = usartData->process_buffer[tail];
        tail = (tail + 1) % USART_PROCESS_BUFFER_SIZE;
        count++;
        if (buffer[count - 1] == usartData->term_symbol)
        {
            usartData->process_tail = tail;
            return count;
        }
    }
    return 0;
}

void
usart_get_errors(USART_Buffer_t *usartData, uint16_t *frame, uint16_t *overrun, uint16_t *parity)
{
    if (frame)
    { *frame = usartData->err_frame; }
    if (overrun)
    { *overrun = usartData->err_overrun; }
    if (parity)
    { *parity = usartData->err_parity; }
}

void
usart_clear_errors(USART_Buffer_t *usartData)
{
    usartData->err_frame   = 0;
    usartData->err_overrun = 0;
    usartData->err_parity  = 0;
}

static void
flush_rx_buffer(USART_Buffer_t *usartData)
{
    usartData->rx_head = 0;
    usartData->rx_tail = 0;
}


ISR(USART_RX_vect)
{
    uint8_t status = UCSR0A;
    uint8_t received_byte = UDR0;
    uint8_t error_flags = 0;

    if (usart_data_ptr)
    {
        if (status & (1 << FE0))
        {
            usart_data_ptr->err_frame++;
            error_flags |= USART_ERROR_FRAME;
        }
        if (status & (1 << DOR0))
        {
            usart_data_ptr->err_overrun++;
            error_flags |= USART_ERROR_OVERRUN;
        }
        if (status & (1 << UPE0))
        {
            usart_data_ptr->err_parity++;
            error_flags |= USART_ERROR_PARITY;
        }

        if (error_flags)
        {
            if (usart_data_ptr->error_callback)
            {
                usart_data_ptr->error_callback(error_flags);
            }
            flush_rx_buffer(usart_data_ptr);
            return;
        }

        if (!is_buffer_full(usart_data_ptr->rx_head, usart_data_ptr->rx_tail, USART_RX_BUFFER_SIZE))
        {
            usart_data_ptr->rx_buffer[usart_data_ptr->rx_head] = received_byte;
            usart_data_ptr->rx_head = (usart_data_ptr->rx_head + 1) % USART_RX_BUFFER_SIZE;
        }

        if (received_byte == usart_data_ptr->term_symbol)
        {
            process_rx_data(usart_data_ptr);
        }
    }
}

