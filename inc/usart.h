#ifndef __USART_H__
#define __USART_H__

#include <stdint.h>

void
usart_init(const uint32_t baudrate);

void
usart_send_char(const char c);

void
usart_send_str(const char *str);

char
usart_recv_char();

#endif /* __USART_H__ */
