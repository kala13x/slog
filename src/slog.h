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

typedef unsigned int            slog_u32_t;
typedef unsigned short int      slog_u16_t;
typedef unsigned char           slog_u8_t;

/* SLog version information */
#define SLOG_VERSION_MAJOR      1
#define SLOG_VERSION_MINOR      8
#define SLOG_BUILD_NUM          27

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
#define SLOG_INDENT             "       "
#define SLOG_SPACE              " "
#define SLOG_EMPTY              ""
#define SLOG_NUL                '\0'

typedef struct SLogDate {
    slog_u16_t nYear;
    slog_u8_t nMonth;
    slog_u8_t nDay;
    slog_u8_t nHour;
    slog_u8_t nMin;
    slog_u8_t nSec;
    slog_u16_t nUsec;
} slog_date_t;

slog_u16_t slog_get_usec();
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
    slog_display(SLOG_NOTAG, __VA_ARGS__)

#define slog_note(...) \
    slog_display(SLOG_NOTE, __VA_ARGS__)

#define slog_info(...) \
    slog_display(SLOG_INFO, __VA_ARGS__)

#define slog_warn(...) \
    slog_display(SLOG_WARN, __VA_ARGS__)

#define slog_debug(...) \
    slog_display(SLOG_DEBUG, __VA_ARGS__)

#define slog_error(...) \
    slog_display(SLOG_ERROR, __VA_ARGS__)

#define slog_trace(...) \
    slog_display(SLOG_TRACE, SLOG_THROW_LOCATION __VA_ARGS__)

#define slog_fatal(...) \
    slog_display(SLOG_FATAL, SLOG_THROW_LOCATION __VA_ARGS__)

/* Short name definitions */
#define slogn(...) slog_note(__VA_ARGS__)
#define slogi(...) slog_info(__VA_ARGS__)
#define slogw(...) slog_warn(__VA_ARGS__)
#define slogd(...) slog_debug( __VA_ARGS__)
#define sloge(...) slog_error( __VA_ARGS__)
#define slogt(...) slog_trace(__VA_ARGS__)
#define slogf(...) slog_fatal(__VA_ARGS__)

typedef struct SLogConfig {
    slog_date_ctrl_t eDateControl;      // Display output with date format
    slog_coloring_t eColorFormat;       // Output color format control
    slog_cb_t logCallback;              // Log callback to collect logs
    void* pCallbackCtx;                 // Data pointer passed to log callback

    slog_u8_t nTraceTid;                // Trace thread ID and display in output
    slog_u8_t nToScreen;                // Enable screen logging
    slog_u8_t nNewLine;                 // Enable new line ending
    slog_u8_t nUseHeap;                 // Use dynamic allocation
    slog_u8_t nToFile;                  // Enable file logging
    slog_u8_t nIndent;                  // Enable indentations
    slog_u8_t nFlush;                   // Flush stdout after screen log
    slog_u16_t nFlags;                  // Allowed log level flags

    char sSeparator[SLOG_NAME_MAX];     // Separator between info and log
    char sFileName[SLOG_NAME_MAX];      // Output file name for logs
    char sFilePath[SLOG_PATH_MAX];      // Output file path for logs
} slog_config_t;

size_t slog_version(char *pDest, size_t nSize, slog_u8_t nMin);
void slog_config_get(slog_config_t *pCfg);
void slog_config_set(slog_config_t *pCfg);

void slog_separator_set(const char *pFormat, ...);
void slog_callback_set(slog_cb_t callback, void *pContext);
void slog_new_line(slog_u8_t nEnable);
void slog_indent(slog_u8_t nEnable);

void slog_enable(slog_flag_t eFlag);
void slog_disable(slog_flag_t eFlag);

void slog_init(const char* pName, slog_u16_t nFlags, slog_u8_t nTdSafe);
void slog_display(slog_flag_t eFlag, const char *pFormat, ...);
void slog_destroy(); // Required only if the slog_init() called with nTdSafe > 0

#ifdef __cplusplus
}
#endif

#endif /* __SLOG_H__ */