/* =============================================================================
 *                          ░▒▓█ _VERTEX_ █▓▒░
 *
 *   File       : serial.h
 *   Author     : MrZloHex
 *   Date       : 2023-05-18
 *
 *   Description:
 *      File contains all the function prototypes and type
 *      definitions for the serial.c file
 *
 * =============================================================================
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "stdio.h"
#include "stdarg.h"

typedef enum USART_Result_E
{
	USART_IO_OK				= 0x00U,
	USART_IO_ERROR			= 0x01U,
	USART_IO_BUSY			= 0x02U,
	USART_IO_TIMEOUT		= 0x03U,
	USART_BUFFER_OVERFLOW	= 0x04U,
} USART_Result;

/**
 * @brief Prints to USART with format
 * 
 * @param size 		Maximum size of message
 * @param format 	Format string
 * @param ... 		va_list of formats
 * @return USART_Result 
 */
USART_Result
uprintf
(
	size_t size,
	const char * restrict format,
	...
);

USART_Result
uvprintf
(
    size_t size,
    const char *restrict format,
    va_list args
);


USART_Result
uputc
(
    const char c
);

/**
 * @brief Reads from USART
 * 
 * @param msg 		Ptr to data buffer
 * @param msg_size 	Size of buffer
 * @param stop_char Char up to which read
 * @return USART_Result 
 */
USART_Result
uread
(
	char *msg,
	size_t msg_size,
	const char stop_char
);

#endif /* __SERIAL_H__ */


#ifdef SERIAL_SEND_IMPL
USART_Result
uprintf
(
	size_t size,
	const char * restrict format,
	...
)
{
	va_list args;
	va_start(args, format);
    USART_Result res = uvprintf(size, format, args);
	va_end(args);

    return res;
}


USART_Result
uvprintf
(
    size_t size,
    const char *restrict format,
    va_list args
)
{

	uint8_t msg[size];
	int size_msg = vsnprintf
	(
		(char *)msg, size,
		format, args
	);

	if (size_msg < 0)
	{
		return USART_BUFFER_OVERFLOW;
	}

	return (USART_Result) SERIAL_SEND_IMPL(msg, size_msg);
}

USART_Result
uputc
(
    const char c
)
{
    return (USART_Result) SERIAL_SEND_IMPL((uint8_t *)&c, 1);
}
#endif

#ifdef SERIAL_RECV_IMPL
USART_Result
uread
(
    char *msg,
	size_t msg_size,
	const char stop_char
)
{
	size_t index = 0;
	uint8_t ch = 0;
	while (1)
	{
		uint8_t result = SERIAL_RECV_IMPL(&ch, 1);

		if (result != USART_IO_OK)
		{
			return (USART_Result) result;
		}

		if (index >= msg_size)
		{
			return USART_BUFFER_OVERFLOW;
		}

		if ((char)ch == stop_char)
		{
			msg[index] = (char)0;
			return USART_IO_OK;
		}
		
		msg[index++] = (char)ch;
	}

	return USART_IO_ERROR;
}
#endif

