#include "protocol.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"
#include "effects.h"
#include "storage.h"
#include "config.h"
#include "util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static app_state_t s_state = APP_UNREG;
static uint32_t    s_last_reg_try = 0;

static char        s_rxline[RX_LINE_MAX];
static uint8_t     s_rxlen = 0;

static nv_state_t  s_nv;

static void
trim(char *s)
{
    /* remove spaces around tokens in-place */
    size_t n = strlen(s);
    while (n && (s[n-1] == ' ' || s[n-1] == '\r' || s[n-1] == '\n')) { s[--n] = '\0'; }
}

static void
send_line(const char *line)
{
    uart_write_str(line);
    uart_write_str("\n");
}

void
proto_send(const char *to, const char *payload)
{
    char buf[96];
    snprintf(buf, sizeof(buf), "%s:%s:%s", to, payload, "VERTEX");
    send_line(buf);
}

void
proto_send_ok(const char *topic)
{
    char pl[48];
    snprintf(pl, sizeof(pl), "%s:OK", topic);
    proto_send("OBELISK", pl);
}

void
proto_send_error(const char *topic, const char *reason)
{
    char pl[64];
    snprintf(pl, sizeof(pl), "%s:ERROR:%s", topic, reason);
    proto_send("OBELISK", pl);
}

void
proto_init(void)
{
    storage_load(&s_nv);
    lamp_set(s_nv.lamp_on);
    effects_init();
    effects_set_mode(s_nv.led.mode);
    effects_set_brightness(s_nv.led.brightness);

    s_state = APP_UNREG;
    s_last_reg_try = 0;
}

app_state_t
proto_get_state(void)
{
    return s_state;
}

void
proto_try_register(void)
{
    char pl[48];
    snprintf(pl, sizeof(pl), "REG:%s", FW_VERSION);
    proto_send("OBELISK", pl);
    s_last_reg_try = millis();
    s_state = APP_WAIT;
}

static void
handle_cmd(char *to, char *payload, char *from)
{
    (void)from;
    trim(to); trim(payload); trim(from);

    if (strcmp(to, "VERTEX") != 0 && strcmp(to, "ALL") != 0)
    {
        return; /* not for us */
    }

    /* Top-level verbs inside payload: VERB[:ARGS] */
    char *verb = strtok(payload, ":");
    char *arg1 = strtok(NULL, ":");
    char *arg2 = strtok(NULL, ":");

    if (!verb)
    {
        proto_send_error("GEN", "EMPTY");
        return;
    }

    if (strcmp(verb, "REG") == 0)
    {
        /* Expect VERTEX:REG:OK:OBELISK or NO */
        if (arg1 && strcmp(arg1, "OK") == 0)
        {
            s_state = APP_READY;
            proto_on_registered_ok();
        }
        else
        {
            s_state = APP_UNREG;
        }
        return;
    }

    if (strcmp(verb, "PING") == 0)
    {
        proto_send("OBELISK", "PONG");
        return;
    }

    if (strcmp(verb, "LAMP") == 0)
    {
        if (!arg1)
        {
            proto_send_error("LAMP", "BAD");
            return;
        }
        if (strcmp(arg1, "ON") == 0)
        {
            lamp_set(1);
            s_nv.lamp_on = 1;
            storage_save(&s_nv);
            proto_send_ok("LAMP");
        }
        else if (strcmp(arg1, "OFF") == 0)
        {
            lamp_set(0);
            s_nv.lamp_on = 0;
            storage_save(&s_nv);
            proto_send_ok("LAMP");
        }
        else if (strcmp(arg1, "TOGGLE") == 0)
        {
            lamp_set(!lamp_get());
            s_nv.lamp_on = lamp_get();
            storage_save(&s_nv);
            proto_send_ok("LAMP");
        }
        else
        {
            proto_send_error("LAMP", "BAD");
        }
        return;
    }

    if (strcmp(verb, "LED") == 0)
    {
        if (!arg1)
        {
            proto_send_error("LED", "BAD");
            return;
        }
        if (strcmp(arg1, "OFF") == 0)
        {
            effects_set_mode(LED_MODE_OFF);
        }
        else if (strcmp(arg1, "SOLID") == 0)
        {
            effects_set_mode(LED_MODE_SOLID);
            if (arg2)
            {
                int v = atoi(arg2);
                if (v < 0) { v = 0; }
                if (v > 255) { v = 255; }
                effects_set_brightness((uint8_t)v);
            }
        }
        else if (strcmp(arg1, "FADE") == 0)
        {
            effects_set_mode(LED_MODE_FADE);
        }
        else if (strcmp(arg1, "BLINK") == 0)
        {
            effects_set_mode(LED_MODE_BLINK);
        }
        else if (strcmp(arg1, "BRIGHT") == 0 && arg2)
        {
            int v = atoi(arg2);
            if (v < 0) { v = 0; }
            if (v > 255) { v = 255; }
            effects_set_brightness((uint8_t)v);
        }
        else
        {
            proto_send_error("LED", "BAD");
            return;
        }
        s_nv.led = effects_get();
        storage_save(&s_nv);
        proto_send_ok("LED");
        return;
    }

    if (strcmp(verb, "BUZZ") == 0)
    {
        /* BUZZ:ON|OFF[:ms] */
        if (!arg1)
        {
            proto_send_error("BUZZ", "BAD");
            return;
        }
        if (strcmp(arg1, "ON") == 0)
        {
            buzzer_set(1);
            proto_send_ok("BUZZ");
        }
        else if (strcmp(arg1, "OFF") == 0)
        {
            buzzer_set(0);
            proto_send_ok("BUZZ");
        }
        else
        {
            proto_send_error("BUZZ", "BAD");
        }
        return;
    }

    proto_send_error("GEN", "UNKNOWN");
}

void
proto_poll(void)
{
    /* Read bytes into line buffer up to \n */
    uint8_t b;
    while (uart_read_byte(&b))
    {
        if (b == '\n')
        {
            s_rxline[s_rxlen] = '\0';
            char *to, *pay, *from;
            if (!parse_packet_alloc(s_rxline, &to, &pay, &from))
            { 
                proto_send_error("PARSE", "FORMAT");
            }
            if (to && pay && from)
            {
                handle_cmd(to, pay, from);
            }
            else
            {
                proto_send_error("GEN", "FORMAT");
            }
            s_rxlen = 0;
            free(to);
            free(pay);
            free(from);
        }
        else
        {
            if (s_rxlen < (RX_LINE_MAX - 1))
            {
                s_rxline[s_rxlen++] = (char)b;
            }
            else
            {
                /* overflow, reset */
                s_rxlen = 0;
                proto_send_error("GEN", "OVF");
            }
        }
    }

    /* Registration / retry */
    if (s_state == APP_UNREG)
    {
        if (elapsed(s_last_reg_try, REG_RETRY_PERIOD_MS))
        {
            proto_try_register();
        }
    }
    else if (s_state == APP_WAIT)
    {
        if (elapsed(s_last_reg_try, REG_ACK_TIMEOUT_MS))
        {
            s_state = APP_UNREG; /* timeout -> try again later */
        }
    }
}

void
proto_on_registered_ok(void)
{
    /* Announce current state */
    char pl[48];
    snprintf(pl, sizeof(pl), "LAMP:%s", lamp_get() ? "ON" : "OFF");
    proto_send("OBELISK", pl);
    snprintf(pl, sizeof(pl), "LED:MODE:%d:BRIGHT:%u", (int)effects_get().mode, effects_get().brightness);
    proto_send("OBELISK", pl);
}
