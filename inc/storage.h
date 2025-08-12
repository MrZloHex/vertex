#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include "effects.h"

typedef struct
{
    uint32_t      magic;        /* 'VX01'                 */
    uint8_t       version;      /* struct version         */
    uint8_t       lamp_on;      /* 0/1                    */
    led_state_t   led;          /* mode + brightness      */
    uint8_t       crc8;         /* Dallas/Maxim poly 0x31 */
} nv_state_t;

void
storage_load(nv_state_t *out);

void
storage_save(const nv_state_t *st);

uint8_t
crc8_dallas(const uint8_t *p, uint8_t len);

#endif /* STORAGE_H */
