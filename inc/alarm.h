#ifndef __ALARM_H__
#define __ALARM_H__

typedef enum
{
    ALARM_MODE_OFF   = 0,
    ALARM_MODE_SOLID = 1,
    ALARM_MODE_BLINK = 2
} AlarmMode;

typedef struct
{
    AlarmMode mode;
    bool      blink;
} Alarm;

void
alarm_init(void);

void
alarm_set_mode(AlarmMode mode);

void
alarm_loop(void);

#endif /* __ALARM_H__ */
