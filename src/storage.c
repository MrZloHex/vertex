#include "storage.h"
#include <avr/eeprom.h>

#define NV_MAGIC 0x31585856UL /* 'VX X1' arbitrary; keep 0x56 'V' low byte */
#define NV_VER   1

static nv_state_t EEMEM ee_state;

static uint8_t
calc_crc(const nv_state_t *st)
{
    return crc8_dallas((const uint8_t *)st, (uint8_t)(sizeof(nv_state_t) - 1));
}

void
storage_load(nv_state_t *out)
{
    nv_state_t tmp;
    eeprom_read_block(&tmp, &ee_state, sizeof(tmp));
    if (tmp.magic != NV_MAGIC || tmp.version != NV_VER)
    {
        out->magic     = NV_MAGIC;
        out->version   = NV_VER;
        out->lamp_on   = 0;
        out->led.mode  = LED_MODE_BLINK;
        out->led.state = 1;
        out->led.brightness = 64;
        out->led.actual_bright = 0;
        out->crc8      = calc_crc(out);
        return;
    }
    uint8_t crc = calc_crc(&tmp);
    if (crc != tmp.crc8)
    {
        out->magic     = NV_MAGIC;
        out->version   = NV_VER;
        out->lamp_on   = 0;
        out->led.mode  = LED_MODE_BLINK;
        out->led.state = 1;
        out->led.brightness = 64;
        out->led.actual_bright = 0;
        out->crc8      = calc_crc(out);
        return;
    }
    *out = tmp;
}

void
storage_save(const nv_state_t *st)
{
    nv_state_t tmp = *st;
    tmp.crc8 = calc_crc(&tmp);
    eeprom_update_block(&tmp, &ee_state, sizeof(tmp));
}

uint8_t
crc8_dallas(const uint8_t *p, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        uint8_t inbyte = p[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) { crc ^= 0x8C; } /* poly 0x31 reflected -> 0x8C */
            inbyte >>= 1;
        }
    }
    return crc;
}
