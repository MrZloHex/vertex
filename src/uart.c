#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#ifndef RX_BUF_SZ
#define RX_BUF_SZ 256
#endif
#ifndef TX_BUF_SZ
#define TX_BUF_SZ 256
#endif

static volatile uint8_t rx_buf[RX_BUF_SZ];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

static volatile uint8_t tx_buf[TX_BUF_SZ];
static volatile uint8_t tx_head = 0;
static volatile uint8_t tx_tail = 0;

void
uart_init(uint32_t baud)
{
    /* UBRR from util/setbaud.h */
    (void)baud; /* compile-time BAUD used */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= (uint8_t)~_BV(U2X0);
#endif
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8N1 */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(UDRIE0);
}

ISR(USART_UDRE_vect)
{
    if (tx_head == tx_tail)
    {
        UCSR0B &= (uint8_t)~_BV(UDRIE0); /* nothing to send */
        return;
    }
    UDR0 = tx_buf[tx_tail];
    tx_tail = (uint8_t)((tx_tail + 1) % TX_BUF_SZ);
}

ISR(USART_RX_vect)
{
    uint8_t data = UDR0;
    uint8_t next = (uint8_t)((rx_head + 1) % RX_BUF_SZ);
    if (next == rx_tail)
    {
        /* overflow, drop oldest */
        rx_tail = (uint8_t)((rx_tail + 1) % RX_BUF_SZ);
    }
    rx_buf[rx_head] = data;
    rx_head         = next;
}

size_t
uart_write(const uint8_t *data, size_t len)
{
    size_t written = 0;
    for (size_t i = 0; i < len; i++)
    {
        uint8_t next;
        do
        {
            uint8_t sreg = SREG;
            cli();
            next = (uint8_t)((tx_head + 1) % TX_BUF_SZ);
            if (next != tx_tail)
            {
                tx_buf[tx_head] = data[i];
                tx_head         = next;
                UCSR0B |= _BV(UDRIE0);
                SREG = sreg;
                written++;
                break;
            }
            SREG = sreg;
        } while (1);
    }
    return written;
}

size_t
uart_write_str(const char *s)
{
    size_t n = 0;
    while (*s)
    {
        n += uart_write((const uint8_t *)s, 1);
        s++;
    }
    return n;
}

int
uart_read_byte(uint8_t *out)
{
    uint8_t sreg = SREG;
    cli();
    if (rx_head == rx_tail)
    {
        SREG = sreg;
        return 0;
    }
    *out   = rx_buf[rx_tail];
    rx_tail = (uint8_t)((rx_tail + 1) % RX_BUF_SZ);
    SREG    = sreg;
    return 1;
}

int
uart_tx_idle(void)
{
    uint8_t sreg = SREG;
    cli();
    int idle = (tx_head == tx_tail);
    SREG = sreg;
    return idle;
}
