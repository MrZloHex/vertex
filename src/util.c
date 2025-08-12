#include "util.h"

#include <string.h>
#include <stddef.h>
#include <stdlib.h>

static char *
dup_range(const char *start, const char *end_excl)
{
    size_t n = (size_t)(end_excl - start);
    char *s = (char *)malloc(n + 1);
    if (!s) { return NULL; }
    if (n)  { memcpy(s, start, n); }
    s[n] = '\0';
    return s;
}

/* Parse "to:payload[:args]:from" into newly-allocated strings.
   Returns true on success. Caller must free *to, *payload, *from. */
bool
parse_packet_alloc(const char *packet, char **to, char **payload, char **from)
{
    if (!packet || !to || !payload || !from) { return false; }

    const char *first = strchr(packet, ':');
    const char *last  = strrchr(packet, ':');

    if (!first || !last || first == last) { return false; } /* need at least 2 colons */

    /* empty fields are allowed? If not, enforce non-empty: */
    if (first == packet) { return false; }                   /* empty 'to' */
    if (*(last + 1) == '\0') { return false; }               /* empty 'from' */

    char *to_s      = dup_range(packet, first);
    char *payload_s = dup_range(first + 1, last);
    char *from_s    = dup_range(last + 1, packet + strlen(packet));

    if (!to_s || !payload_s || !from_s)
    {
        free(to_s);
        free(payload_s);
        free(from_s);
        return false;
    }

    *to      = to_s;
    *payload = payload_s;  /* includes any internal ':' between first and last */
    *from    = from_s;
    return true;
}
