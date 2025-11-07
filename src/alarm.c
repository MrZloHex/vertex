#include "alarm.h"
#include "gpio.h"
#include "timer.h"

static Alarm s_alarm = { .mode = ALARM_MODE_OFF };
static Timer s_alarm_tim;

void
alarm_init(void)
{
    s_alarm.mode = ALARM_MODE_OFF;
    s_alarm.blink = false;
    buzzer_set(false);
    timer_set(&s_alarm_tim, 500, true);
}

void
alarm_set_mode(AlarmMode mode)
{
    s_alarm.mode = mode;
    if (mode == ALARM_MODE_OFF)
    {
        buzzer_set(false);
    }
    else if (mode == ALARM_MODE_SOLID)
    {
        buzzer_set(true);
    }
    else
    {
        timer_start(&s_alarm_tim);
        s_alarm.blink = true;
    }
}

void
alarm_loop(void)
{
    if (!timer_timeout(&s_alarm_tim))
    { return; }

    switch (s_alarm.mode)
    {
        case ALARM_MODE_OFF:
        case ALARM_MODE_SOLID:
            /* nothing */
            break;
        case ALARM_MODE_BLINK:
        {
            buzzer_set(s_alarm.blink);
            s_alarm.blink = !s_alarm.blink;
        } break;
    }
}

