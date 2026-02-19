#include "protocol.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"
#include "led.h"
#include "alarm.h"
#include "storage.h"
#include "config.h"
#include "util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static app_state_t s_state = APP_UNREG;

static char        s_rxline[RX_LINE_MAX];
static uint16_t    s_rxlen = 0;

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
proto_send_ok(const char *topic, const char *to)
{
    char pl[48];
    snprintf(pl, sizeof(pl), "OK:%s", topic);
    proto_send(to, pl);
}

void
proto_send_error(const char *topic, const char *reason, const char *to)
{
    char pl[64];
    snprintf(pl, sizeof(pl), "ERR:%s:%s", topic, reason);
    proto_send(to, pl);
}

void
proto_init(void)
{
    storage_load(&s_nv);
    lamp_set(s_nv.lamp_on);
    effects_init();
    effects_set_mode(s_nv.led.mode);
    effects_set_state(s_nv.led.state);
    effects_set_brightness(s_nv.led.brightness);

    s_state = APP_READY;
    proto_send("ALL", "REG:VERTEX");
}


app_state_t
proto_get_state(void)
{
    return s_state;
}

static void
handle_cmd(char *to, char *payload, char *from)
{
    trim(to); trim(payload); trim(from);

    if (strcmp(to, "VERTEX") != 0 && strcmp(to, "ALL") != 0)
    {
        return; /* not for us */
    }

    /* Top-level verbs inside payload: VERB:NOUN[:ARGS] */
    char *verb = strtok(payload, ":");
    char *noun = strtok(NULL, ":");
    char *arg1 = strtok(NULL, ":");
    char *arg2 = strtok(NULL, ":");

    if (!verb)
    {
        proto_send_error("VERB", "EMPTY", from);
        return;
    }

    if (strcmp(verb, "PING") == 0)
    {
        proto_send(from, "PONG:PONG");
        return;
    }

    if (!noun)
    {
        proto_send_error("NOUN", "EMPTY", from);
        return;
    }

    if (strcmp(verb, "ON") == 0)
    {
        if (strcmp(noun, "LAMP") == 0)
        {
            lamp_set(0);
            s_nv.lamp_on = 0;
            storage_save(&s_nv);
            proto_send_ok("LAMP", from);
        }
        else if (strcmp(noun, "LED") == 0)
        {
            effects_set_state(1);
            s_nv.led = effects_get();
            storage_save(&s_nv);
            proto_send_ok("LED", from);
        }
        else if (strcmp(noun, "BUZZ") == 0)
        {
            buzzer_set(1);
            proto_send_ok("BUZZ", from);
        }
        else
        {
            proto_send_error("NOUN", "UNK", from);
        }
    }
    else if (strcmp(verb, "OFF") == 0)
    {
        if (strcmp(noun, "LAMP") == 0)
        {
            lamp_set(1);
            s_nv.lamp_on = 1;
            storage_save(&s_nv);
            proto_send_ok("LAMP", from);
        }
        else if (strcmp(noun, "LED") == 0)
        {
            effects_set_state(0);
            s_nv.led = effects_get();
            storage_save(&s_nv);
            proto_send_ok("LED", from);
        }
        else if (strcmp(noun, "BUZZ") == 0)
        {
            buzzer_set(0);
            proto_send_ok("BUZZ", from);
        }
        else
        {
            proto_send_error("NOUN", "UNK", from);
        }
    }
    else if (strcmp(verb, "TOGGLE") == 0)
    {
        if (strcmp(noun, "LAMP") == 0)
        {
            lamp_set(!lamp_get());
            s_nv.lamp_on = lamp_get();
            storage_save(&s_nv);
            proto_send_ok("LAMP", from);
        }
        else if (strcmp(noun, "LED") == 0)
        {
            led_state_t st = effects_get();
            effects_set_state(!st.state);
            s_nv.led = effects_get();
            storage_save(&s_nv);
            proto_send_ok("LED", from);
        }
        else
        {
            proto_send_error("NOUN", "UNK", from);
        }
    }
    else if (strcmp(verb, "SET") == 0)
    {
        if (strcmp(noun, "LED") == 0)
        {
            if (!arg1)
            { proto_send_error("ARG1", "EMPTY", from); return; }
            if (!arg2)
            { proto_send_error("ARG2", "EMPTY", from); return; }

            if (strcmp(arg1, "MODE") == 0)
            {
                if (strcmp(arg2, "BLINK") == 0)
                { effects_set_mode(LED_MODE_BLINK); }
                else if (strcmp(arg2, "SOLID") == 0)
                { effects_set_mode(LED_MODE_SOLID); }
                else if (strcmp(arg2, "FADE") == 0)
                { effects_set_mode(LED_MODE_FADE); }
                else
                { proto_send_error("LED:MODE", "UNK", from); return; }
            }
            else if (strcmp(arg1, "BRIGHT") == 0)
            {
                int v = atoi(arg2);
                if (v < 0) { v = 0; }
                if (v > 255) { v = 255; }
                effects_set_brightness((uint8_t)v);
            }
            else
            {
                proto_send_error("LED", "UNK", from);
                return;
            }

            s_nv.led = effects_get();
            storage_save(&s_nv);
            proto_send_ok("LED", from);
        }
        else
        {
            proto_send_error("NOUN", "UNK", from);
        }
    }
    else if (strcmp(verb, "GET") == 0)
    {
        if (strcmp(noun, "LAMP") == 0)
        {
            if (!arg1)
            { proto_send_error("ARG1", "EMPTY", from); return; }

            if (strcmp(arg1, "STATE") == 0)
            {
                if (!lamp_get())
                { proto_send_ok("LAMP:STATE:ON", from); }
                else
                { proto_send_ok("LAMP:STATE:OFF", from); }
            }
            else
            {
                proto_send_error("LAMP", "UNK", from);
            }
        }
        else if (strcmp(noun, "LED") == 0)
        {
            if (!arg1)
            { proto_send_error("ARG1", "EMPTY", from); return; }

            led_state_t state = effects_get();

            if (strcmp(arg1, "MODE") == 0)
            {
                if (state.mode == LED_MODE_FADE)
                { proto_send_ok("LED:MODE:FADE", from); }
                else if (state.mode == LED_MODE_BLINK)
                { proto_send_ok("LED:MODE:BLINK", from); }
                else if (state.mode == LED_MODE_SOLID)
                { proto_send_ok("LED:MODE:SOLID", from); }
            }
            else if (strcmp(arg1, "STATE") == 0)
            {
                if (state.state)
                { proto_send_ok("LED:STATE:ON", from); }
                else
                { proto_send_ok("LED:STATE:OFF", from); }
            }
            else if (strcmp(arg1, "BRIGHT") == 0)
            {
                char pl[48];
                snprintf(pl, sizeof(pl), "OK:LED:BRIGHT:%d", state.brightness);
                proto_send(from, pl);
            }
            else
            {
                proto_send_error("LED", "UNK", from);
            }
        }
        else if (strcmp(noun, "UPTIME") == 0)
        {
            char pl[48];
            snprintf(pl, sizeof(pl), "OK:UPTIME:%lu", (unsigned long)timer_now());
            proto_send(from, pl);
        }
        else
        {
            proto_send_error("NOUN", "UNK", from);
        }
    }
    else
    {
        proto_send_error("VERB", "UNK", from);
    }
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
            char *to = NULL, *pay = NULL, *from = NULL;
            if (!parse_packet_alloc(s_rxline, &to, &pay, &from))
            { 
                proto_send_error("PROTO", "FORMAT", "ALL");
            }
            else if (to && pay && from)
            {
                handle_cmd(to, pay, from);
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
                proto_send_error("GEN", "OVF", "ALL");
            }
        }
    }
}

