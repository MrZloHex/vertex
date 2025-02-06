#ifndef __TRACE_H__
#define __TRACE_H__

/**
 * @file   trace.h
 * @author Zlobin Aleksey
 * @brief  A library for logging and profiling on STM32 microcontrollers.
 *
 * This library provides a universal mechanism for outputting debug information,
 * measuring code execution time using DWT (Data Watchpoint and Trace),
 * as well as optional caching of logs in MRAM memory (if available) for subsequent analysis.
 *
 * Possible output channels:
 * - SWO  – for STM32 chips with SWO support.
 * - UART – output to a serial port.
 * - MRAM – saving logs to MRAM.
 *
 * Profiling capabilities include measuring the number of CPU cycles
 * and converting them to microseconds and nanoseconds.
 *
 * @note The corresponding hardware settings and initialization are required, for example:
 *       - UART initialization
 *       - SWO configuration on the debug adapter
 *       - MRAM initialization (if caching is used)
 *       - Enabling the DWT counter (if profiling is used)
 */


/**
 * Logging and profiling configuration
 *
 * @def ENABLE_DWT
 * @brief Enable DWT (Data Watchpoint and Trace) support for measuring CPU cycles.
 *
 * @def SWO_TRACING
 * @brief Enable log output via SWO.
 *
 * @def UART_TRACING
 * @brief Enable log output via UART.
 *
 * @def MRAM_CACHING
 * @brief Enable log caching in MRAM.
 *
 * @def MRAM_BASE_ADDR
 * @brief Base address for writing logs to MRAM.
 *
 */

#undef  ENABLE_DWT
#undef  SWO_TRACING
#define UART_TRACING
#undef  MRAM_CACHING
#define MRAM_BASE_ADDR 0x0


#ifdef UART_TRACING
#   include "serial.h"
#endif

#ifdef MRAM_CACHING
#   include "mram.h"
#endif

#ifdef ENABLE_DWT
#   if   (__CORTEX_M == 3)
#      include "core_cm3.h"
#   elif (__CORTEX_M == 4)
#      include "core_cm4.h"
#   elif (__CORTEX_M == 7)
#      include "core_cm7.h"
#   else
#      error "NO DWT SUPPORT"
#   endif
#endif /* ENABLE_DWT */

typedef enum
{
    TRC_DEBUG,
    TRC_INFO,
    TRC_WARN,
    TRC_ERROR,
    TRC_FATAL
} Trace_Level;

/**
 * The maximum length of the formatted log string.
 */
#define MAX_ENTRY_DSCR_LEN 512

typedef struct
{
    Trace_Level lvl;
    char fmt[MAX_ENTRY_DSCR_LEN];
} Trace_Entry;

typedef enum
{
    TP_FILE = 0b0001,
    TP_FUNC = 0b0010,
    TP_LINE = 0b0100,
    TP_TIME = 0b1000,
    TP_ALL  = 0b1111
} Trace_Params;

/** @brief A macro for checking output parameters. */
#define IS_ENABLED(__TP__) \
    (tracer.params & __TP__)

typedef struct Tracer_S
{
    Trace_Level         base_level;
    Trace_Params        params;
#ifdef UART_TRACING
	// UART_HandleTypeDef *huart;
#endif
#ifdef MRAM_CACHING
    MRAM               *mram;
    size_t              addr;
    uint32_t            entry_quantity;
#endif
#ifdef ENABLE_DWT
    uint32_t            cpu_freq;
#endif
} Tracer;

#define __FILENAME__ (__builtin_strrchr("/"__FILE__, '/') + 1)


/**
 * @brief   Macros for quick logging of messages of various levels.
 * @details Automatically substitutes file name, function name, and line number.
 * @see     tracer_trace()
 */
#define TRACE_DEBUG(...) tracer_trace(TRC_DEBUG, __FILENAME__, __func__, __LINE__, __VA_ARGS__)
#define TRACE_INFO(...)  tracer_trace(TRC_INFO , __FILENAME__, __func__, __LINE__, __VA_ARGS__)
#define TRACE_WARN(...)  tracer_trace(TRC_WARN , __FILENAME__, __func__, __LINE__, __VA_ARGS__)
#define TRACE_ERROR(...) tracer_trace(TRC_ERROR, __FILENAME__, __func__, __LINE__, __VA_ARGS__)
#define TRACE_FATAL(...) tracer_trace(TRC_FATAL, __FILENAME__, __func__, __LINE__, __VA_ARGS__)


/**
 * @brief Logger initialization.
 *
 * @param[in] mram   Pointer to the MRAM structure (if caching is used).
 * @param[in] huart  UART handle (if UART output is used).
 * @param[in] level  Base logging level.
 * @param[in] params Output parameters (file name, function, line).
 *
 * @note Parameters depend on the enabled macros (UART_TRACING, MRAM_CACHING).
 */
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
);

/**
 * @brief Set the base logging level.
 *
 * Messages with a level below the specified one will not be displayed.
 *
 * @param level The new logging level.
 */
void
tracer_set_level(Trace_Level level);

#ifdef MRAM_CACHING
static inline size_t
_tracer_cacl_addr(size_t n)
{
    return MRAM_BASE_ADDR + 1 + (MAX_ENTRY_DSCR_LEN +1) *n;
}

/**
 * @brief Dump all entries from MRAM (when caching is enabled).
 *
 * Outputs all saved logs, filtering them by level.
 *
 * @param[in] filter_level The minimum log level to output.
 */
void
tracer_dump(Trace_Level filter_level);

/**
 * @brief Erase all logs from MRAM.
 *
 * Resets the log counter and clears the corresponding MRAM area.
 */
void
tracer_erase();
#endif

#ifdef SWO_TRACING
void
_tracer_swo_print(Trace_Entry entry);
#endif

#ifdef UART_TRACING
void
_tracer_usart_print(Trace_Entry entry);
#endif

/**
 * @brief Output a log message with a given level and format.
 *
 * @param[in] level The log message level.
 * @param[in] file  The name of the file in which the message is formed.
 * @param[in] func  The name of the function.
 * @param[in] line  The line number in the file.
 * @param[in] fmt   Formatted string and associated parameters.
 */
void
tracer_trace
(
    Trace_Level level, const char *restrict file,
    const char *restrict func, int line, const char *fmt, ...
);

#ifdef ENABLE_DWT

/**
 * @brief Macro for measuring function execution time in cycles.
 *
 * @param[in] __RET__       Variable for storing the result of the function call.
 * @param[in] __FUNC_CALL__ The called function (or expression).
 *
 * Usage example:
 * @code
 *   int res;
 *   uint32_t cycles = MEASURE(res, some_function());
 *   TRACE_INFO("some_function took %u cycles", cycles);
 * @endcode
 */
#define MEASURE(__RET__, __FUNC_CALL__)       \
    ({                                        \
        uint32_t start = tracer_get_cycles(); \
        __RET__ = __FUNC_CALL__;              \
        uint32_t end   = tracer_get_cycles(); \
        (end - start);                        \
    })


/**
 * @brief Get the current value of the DWT cycle counter.
 * @return The current value of CYCCNT.
 */
uint32_t
tracer_get_cycles(void);

/**
 * @brief Convert the number of cycles to microseconds.
 *
 * @param[in] cycles The number of CPU cycles.
 * @return The time in microseconds.
 */
uint32_t
tracer_get_us(uint32_t cycles);

/**
 * @brief Convert the number of cycles to nanoseconds.
 *
 * @param[in] cycles The number of CPU cycles.
 * @return The time in nanoseconds.
 */
uint32_t
tracer_get_ns(uint32_t cycles);


#endif

#endif /* __TRACE_H__ */

