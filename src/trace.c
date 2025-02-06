/**
 * @file   trace.c
 * @author Zlobin Aleksey
 * @brief  Implementation of logging and profiling functions.
 *
 * This file contains the implementation of the functions declared in trace.h, including:
 * - Logger initialization
 * - Outputting messages to various channels (SWO, UART)
 * - Caching logs in MRAM (when enabled)
 * - Measuring the runtime of functions and converting it to microseconds/nanoseconds
 */

#include "trace.h"
#include "stdarg.h"
#include "stdio.h"
#include "tick.h"


static const char *const trace_levels[] =
{
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static Tracer tracer;

void
tracer_init
(
#ifdef MRAM_CACHING
    MRAM *mram,
#endif
#ifdef UART_TRACING
	// UART_HandleTypeDef *huart,
#endif
    Trace_Level level, Trace_Params params
)
{
    tracer = (Tracer)
    {
        .base_level = level,
        .params     = params,
#ifdef UART_TRACING
    //     .huart      = huart,
#endif
#ifdef ENABLE_DWT
        .cpu_freq   = SystemCoreClock,
#endif
    };
#ifdef MRAM_CACHING
    tracer.mram = mram;
    mram_read_dword(mram, MRAM_BASE_ADDR, &tracer.entry_quantity);
    tracer.addr = _tracer_cacl_addr(tracer.entry_quantity);
#endif
#ifdef ENABLE_DWT
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0)
    { CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; }

    DWT->LAR    = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
#endif
}

void
tracer_set_level(Trace_Level level);

#ifdef MRAM_CACHING
void
tracer_dump(Trace_Level filter_level)
{
    for (size_t i = 0; i <= tracer.entry_quantity; ++i)
    {
        Trace_Entry entry;
        mram_read_array(tracer.mram, _tracer_cacl_addr(i), (uint8_t *)entry.fmt, MAX_ENTRY_DSCR_LEN);
        mram_read_byte(tracer.mram, _tracer_cacl_addr(i) + MAX_ENTRY_DSCR_LEN, &(entry.lvl));
         
        if (entry.lvl < filter_level)
        { continue; }
#ifdef SWO_TRACING
        _tracer_swo_print(entry);
#endif
#ifdef UART_TRACING
        _tracer_usart_print(entry);
#endif
    }
}

void
tracer_eraser()
{
    mram_write_byte(tracer.mram, MRAM_BASE_ADDR, 0);
    tracer.entry_quantity = 0;
    tracer.addr = (MAX_ENTRY_DSCR_LEN + 1) *tracer.entry_quantity;
}

void
_tracer_cache_entry(Trace_Entry entry)
{
    HAL_StatusTypeDef res = mram_write_array(tracer.mram, tracer.addr, (uint8_t *)entry.fmt, MAX_ENTRY_DSCR_LEN);
    res |= mram_write_byte(tracer.mram, tracer.addr + MAX_ENTRY_DSCR_LEN, entry.lvl);

    TRACE_DEBUG("MRAM CACHE %u at %u", res, tracer.addr);
    if (res != HAL_OK)
    { TRACE_ERROR("FAILED TO CACHE TRACE"); }

    tracer.addr += MAX_ENTRY_DSCR_LEN + 1;
    tracer.entry_quantity += 1;
}

#endif

#ifdef SWO_TRACING
void
_tracer_swo_print(Trace_Entry entry)
{
    printf("%s\n", entry.fmt);
}
#endif

#ifdef UART_TRACING
void
_tracer_usart_print(Trace_Entry entry)
{
    uprintf(200, "%s\n", entry.fmt);
}
#endif

void
tracer_trace
(
    Trace_Level level, const char *restrict file,
    const char *restrict func, int line, const char *fmt, ...
)
{
    if (tracer.base_level > level)
    { return; }

    va_list args;
    va_start(args, fmt);
    

    Trace_Entry entry = { .lvl  = level };
    int pc = 0;

    if (IS_ENABLED(TP_TIME))
    { pc += snprintf(entry.fmt, MAX_ENTRY_DSCR_LEN, "%lu", avr_get_tick()); }
    pc += snprintf(entry.fmt + pc, MAX_ENTRY_DSCR_LEN - pc, " %5s ", trace_levels[level]);


    if (IS_ENABLED(TP_FILE))
    { pc += snprintf(entry.fmt + pc, MAX_ENTRY_DSCR_LEN - pc, "%s:", file); }
    if (IS_ENABLED(TP_FUNC))
    { pc += snprintf(entry.fmt + pc, MAX_ENTRY_DSCR_LEN - pc, "%s:", func); }
    if (IS_ENABLED(TP_LINE))
    { pc += snprintf(entry.fmt + pc, MAX_ENTRY_DSCR_LEN - pc, "%d: ", line); }

    pc += vsnprintf(entry.fmt + pc, MAX_ENTRY_DSCR_LEN - pc, fmt, args);
    va_end(args);


#ifdef SWO_TRACING
    _tracer_swo_print(entry);
#endif

#ifdef UART_TRACING
    _tracer_usart_print(entry);
#endif

#ifdef MRAM_CACHING
    _tracer_cache_entry(entry);
#endif

}

#ifdef ENABLE_DWT
uint32_t
tracer_get_cycles(void)
{ return DWT->CYCCNT; }

uint32_t
tracer_get_us(uint32_t cycles)
{ return (uint32_t)(((uint64_t)cycles * 1000000ULL)    / tracer.cpu_freq); }

uint32_t
tracer_get_ns(uint32_t cycles)
{ return (uint32_t)(((uint64_t)cycles * 1000000000ULL) / tracer.cpu_freq); }

#endif


