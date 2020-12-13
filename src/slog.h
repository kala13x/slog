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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <inttypes.h>
#include <pthread.h>

/* SLog version information */
#define SLOG_VERSION_MAJOR  1
#define SLOG_VERSION_MINOR  8
#define SLOG_BUILD_NUM      22

/* Supported colors */
#define SLOG_CLR_NORMAL     "\x1B[0m"
#define SLOG_CLR_RED        "\x1B[31m"
#define SLOG_CLR_GREEN      "\x1B[32m"
#define SLOG_CLR_YELLOW     "\x1B[33m"
#define SLOG_CLR_BLUE       "\x1B[34m"
#define SLOG_CLR_MAGENTA    "\x1B[35m"
#define SLOG_CLR_CYAN       "\x1B[36m"
#define SLOG_CLR_WHITE      "\x1B[37m"
#define SLOG_CLR_RESET      "\033[0m"

/* Trace source location helpers */
#define SLOG_TRACE_LVL1(LINE) #LINE
#define SLOG_TRACE_LVL2(LINE) SLOG_TRACE_LVL1(LINE)
#define SLOG_THROW_LOCATION "[" __FILE__ ":" SLOG_TRACE_LVL2(__LINE__) "] "

/* SLog limits (To be safe while avoiding dynamic allocations) */
#define SLOG_MESSAGE_MAX    8196
#define SLOG_VERSION_MAX    128
#define SLOG_PATH_MAX       2048
#define SLOG_NAME_MAX       256
#define SLOG_DATE_MAX       64
#define SLOG_TAG_MAX        32

#define SLOG_FLAGS_CHECK(c, f) (((c) & (f)) == (f))
#define SLOG_FLAGS_ALL      255

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
} SLOG_FLAGS_E;

/* Output coloring control flags */
typedef enum
{
    SLOG_COLOR_DISABLE = 0,
    SLOG_COLOR_TAG,
    SLOG_COLOR_FULL
} SLOG_COLOR_FMT_E;

#define slog(...) \
    slog_print(SLOG_NOTAG, 1, __VA_ARGS__)

#define slogwn(...) \
    slog_print(SLOG_NOTAG, 0, __VA_ARGS__)

#define slog_note(...) \
    slog_print(SLOG_NOTE, 1, __VA_ARGS__)

#define slog_info(...) \
    slog_print(SLOG_INFO, 1, __VA_ARGS__)

#define slog_warn(...) \
    slog_print(SLOG_WARN, 1, __VA_ARGS__)

#define slog_debug(...) \
    slog_print(SLOG_DEBUG, 1, __VA_ARGS__)

#define slog_error(...) \
    slog_print(SLOG_ERROR, 1, __VA_ARGS__)

#define slog_trace(...) \
    slog_print(SLOG_TRACE, 1, SLOG_THROW_LOCATION __VA_ARGS__)

#define slog_fatal(...) \
    slog_print(SLOG_FATAL, 1, SLOG_THROW_LOCATION __VA_ARGS__)

typedef struct SLogConfig {
    char sFileName[SLOG_NAME_MAX];      // Output file name for logs
    char sFilePath[SLOG_PATH_MAX];      // Output file path for logs
    SLOG_COLOR_FMT_E eColorFormat;      // Output color format control
    uint8_t nTraceTid:1;                // Trace thread ID and display in output
    uint8_t nToScreen:1;                // Enable screen logging
    uint8_t nToFile:1;                  // Enable file logging
    uint8_t nFlush:1;                   // Flush stdout after screen log
    uint16_t nFlags;                    // Allowed log level flags
} SLogConfig;

const char* slog_version(uint8_t nMin);
void slog_config_get(SLogConfig *pCfg);
void slog_config_set(SLogConfig *pCfg);

void slog_enable(SLOG_FLAGS_E eFlag);
void slog_disable(SLOG_FLAGS_E eFlag);

void slog_init(const char* pName, uint16_t nFlags, uint8_t nTdSafe);
void slog_print(SLOG_FLAGS_E eFlag, uint8_t nNewLine, const char *pMsg, ...);
void slog_destroy(); // Needed only if the slog_init() function argument nTdSafe > 0

#ifdef __cplusplus
}
#endif

#endif /* __SLOG_H__ */