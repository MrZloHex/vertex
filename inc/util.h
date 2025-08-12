#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdbool.h>

bool
parse_packet_alloc(const char *packet, char **to, char **payload, char **from);

#endif /* __UTIL_H__ */
