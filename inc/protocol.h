#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/* Simple colon-separated protocol with newline terminator:
 * <TO>:<PAYLOAD>:<FROM>\n
 */

typedef enum
{
    APP_UNREG  = 0,
    APP_WAIT   = 1,
    APP_READY  = 2
} app_state_t;

void
proto_init(void);

void
proto_poll(void); /* consume RX, process lines, emit replies */

void
proto_try_register(void); /* send REG packet */

void
proto_on_registered_ok(void);

app_state_t
proto_get_state(void);

void
proto_send(const char *to, const char *payload);

/* helpers for building replies */
void
proto_send_ok(const char *topic);

void
proto_send_error(const char *topic, const char *reason);

#endif /* PROTOCOL_H */
