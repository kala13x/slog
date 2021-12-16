/*
 * The MIT License (MIT)
 *
 *  Copyleft (C) 2015-2020  Sun Dro (f4tb0y@protonmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE
 */

#ifndef __SLOG_H__
#define __SLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <pthread.h>

/* SLog version information */
#define SLOG_VERSION_MAJOR      1
#define SLOG_VERSION_MINOR      8
#define SLOG_BUILD_NUM          25

/* Supported colors */
#define SLOG_COLOR_NORMAL       "\x1B[0m"
#define SLOG_COLOR_RED          "\x1B[31m"
#define SLOG_COLOR_GREEN        "\x1B[32m"
#define SLOG_COLOR_YELLOW       "\x1B[33m"
#define SLOG_COLOR_BLUE         "\x1B[34m"
#define SLOG_COLOR_MAGENTA      "\x1B[35m"
#define SLOG_COLOR_CYAN         "\x1B[36m"
#define SLOG_COLOR_WHITE        "\x1B[37m"
#define SLOG_COLOR_RESET        "\033[0m"

/* Trace source location helpers */
#define SLOG_TRACE_LVL1(LINE) #LINE
#define SLOG_TRACE_LVL2(LINE) SLOG_TRACE_LVL1(LINE)
#define SLOG_THROW_LOCATION "[" __FILE__ ":" SLOG_TRACE_LVL2(__LINE__) "] "

/* SLog limits (To be safe while avoiding dynamic allocations) */
#define SLOG_MESSAGE_MAX        8196
#define SLOG_VERSION_MAX        128
#define SLOG_PATH_MAX           2048
#define SLOG_INFO_MAX           512
#define SLOG_NAME_MAX           256
#define SLOG_DATE_MAX           64
#define SLOG_TAG_MAX            32
#define SLOG_COLOR_MAX          16

#define SLOG_FLAGS_CHECK(c, f) (((c) & (f)) == (f))
#define SLOG_FLAGS_ALL          255

#define SLOG_NAME_DEFAULT       "slog"
#define SLOG_NEWLINE            "\n"
#define SLOG_EMPTY              ""
#define SLOG_NUL                '\0'

typedef struct SLogDate {
    uint16_t nYear;
    uint8_t nMonth;
    uint8_t nDay;
    uint8_t nHour;
    uint8_t nMin;
    uint8_t nSec;
    uint8_t nUsec;
} slog_date_t;

uint8_t slog_get_usec();
void slog_get_date(slog_date_t *pDate);

/* Log level flags */
typedef enum
{
    SLOG_NOTAG = (1 << 0),
    SLOG_NOTE = (1 << 1),
    SLOG_INFO = (1 << 2),
    SLOG_WARN = (1 << 3),
    SLOG_DEBUG = (1 << 4),
    SLOG_TRACE = (1 << 5),
    SLOG_ERROR = (1 << 6),
    SLOG_FATAL = (1 << 7)
} slog_flag_t;

typedef int(*slog_cb_t)(const char *pLog, size_t nLength, slog_flag_t eFlag, void *pCtx);

/* Output coloring control flags */
typedef enum
{
    SLOG_COLORING_DISABLE = 0,
    SLOG_COLORING_TAG,
    SLOG_COLORING_FULL
} slog_coloring_t;

typedef enum
{
    SLOG_TIME_DISABLE = 0,
    SLOG_TIME_ONLY,
    SLOG_DATE_FULL
} slog_date_ctrl_t;

#define slog(...) \
    slog_display(SLOG_NOTAG, 1, __VA_ARGS__)

#define slogwn(...) \
    slog_display(SLOG_NOTAG, 0, __VA_ARGS__)

#define slog_note(...) \
    slog_display(SLOG_NOTE, 1, __VA_ARGS__)

#define slog_info(...) \
    slog_display(SLOG_INFO, 1, __VA_ARGS__)

#define slog_warn(...) \
    slog_display(SLOG_WARN, 1, __VA_ARGS__)

#define slog_debug(...) \
    slog_display(SLOG_DEBUG, 1, __VA_ARGS__)

#define slog_error(...) \
    slog_display(SLOG_ERROR, 1, __VA_ARGS__)

#define slog_trace(...) \
    slog_display(SLOG_TRACE, 1, SLOG_THROW_LOCATION __VA_ARGS__)

#define slog_fatal(...) \
    slog_display(SLOG_FATAL, 1, SLOG_THROW_LOCATION __VA_ARGS__)

/* Short name definitions */
#define slogn(...) slog_note(__VA_ARGS__)
#define slogi(...) slog_info(__VA_ARGS__)
#define slogw(...) slog_warn(__VA_ARGS__)
#define slogd(...) slog_debug( __VA_ARGS__)
#define sloge(...) slog_error( __VA_ARGS__)
#define slogt(...) slog_trace(__VA_ARGS__)
#define slogf(...) slog_fatal(__VA_ARGS__)

typedef struct SLogConfig {
    slog_date_ctrl_t eDateControl;     // Display output with date format
    slog_coloring_t eColorFormat;      // Output color format control
    slog_cb_t logCallback;             // Log callback to collect logs
    void* pCallbackCtx;                // Data pointer passed to log callback

    uint8_t nTraceTid:1;                // Trace thread ID and display in output
    uint8_t nToScreen:1;                // Enable screen logging
    uint8_t nUseHeap:1;                 // Use dynamic allocation
    uint8_t nToFile:1;                  // Enable file logging
    uint8_t nFlush:1;                   // Flush stdout after screen log
    uint16_t nFlags;                    // Allowed log level flags

    char sSeparator[SLOG_NAME_MAX];     // Separator between info and log
    char sFileName[SLOG_NAME_MAX];      // Output file name for logs
    char sFilePath[SLOG_PATH_MAX];      // Output file path for logs
} slog_config_t;

size_t slog_version(char *pDest, size_t nSize, uint8_t nMin);
void slog_config_get(slog_config_t *pCfg);
void slog_config_set(slog_config_t *pCfg);

void slog_separator_set(const char *pFormat, ...);
void slog_callback_set(slog_cb_t callback, void *pContext);

void slog_enable(slog_flag_t eFlag);
void slog_disable(slog_flag_t eFlag);

void slog_init(const char* pName, uint16_t nFlags, uint8_t nTdSafe);
void slog_display(slog_flag_t eFlag, uint8_t nNewLine, const char *pFormat, ...);
void slog_destroy(); // Needed only if the slog_init() function argument nTdSafe > 0

#ifdef __cplusplus
}
#endif

#endif /* __SLOG_H__ */