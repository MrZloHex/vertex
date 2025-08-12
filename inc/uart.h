#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>

/* Non-blocking UART with IRQ-driven TX/RX ring buffers. */

void
uart_init(uint32_t baud);

size_t
uart_write(const uint8_t *data, size_t len);

size_t
uart_write_str(const char *s);

int
uart_read_byte(uint8_t *out); /* returns 1 if a byte was read, 0 if none */

int
uart_tx_idle(void);

#endif /* UART_H */
