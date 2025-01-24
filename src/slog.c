/*
 * The MIT License (MIT)
 *
 *  Copyleft (C) 2015-2023  Sun Dro (s.kalatoz@gmail.com)
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "slog.h"

#ifdef __linux__
#include <syscall.h>
#endif

#include <sys/time.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifndef PTHREAD_MUTEX_RECURSIVE 
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

typedef struct slog_file {
    uint8_t nCurrDay;
    FILE *pHandle;
} slog_file_t;

typedef struct slog {
    pthread_mutex_t mutex;
    slog_config_t config;
    slog_file_t logFile;
    uint8_t nTdSafe;
} slog_t;

typedef struct slog_context {
    const char *pFormat;
    slog_flag_t eFlag;
    slog_date_t date;
    uint8_t nNewLine;
} slog_context_t;

static volatile int g_nHaveSlogVerShort = 0;
static volatile int g_nHaveSlogVerLong = 0;
static char g_slogVerShort[128];
static char g_slogVerLong[256];

static slog_t g_slog;

static void slog_sync_init(slog_t *pSlog)
{
    if (!pSlog->nTdSafe) return;
    pthread_mutexattr_t mutexAttr;

    if (pthread_mutexattr_init(&mutexAttr) ||
        pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE) ||
        pthread_mutex_init(&pSlog->mutex, &mutexAttr) ||
        pthread_mutexattr_destroy(&mutexAttr))
    {
        printf("<%s:%d> %s: [ERROR] Can not initialize mutex: %d\n", 
            __FILE__, __LINE__, __func__, errno);

        exit(EXIT_FAILURE);
    }
}

static void slog_lock(slog_t *pSlog)
{
    if (pSlog->nTdSafe && pthread_mutex_lock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not lock mutex: %d\n", 
            __FILE__, __LINE__, __func__, errno);

        exit(EXIT_FAILURE);
    }
}

static void slog_unlock(slog_t *pSlog)
{
    if (pSlog->nTdSafe && pthread_mutex_unlock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not unlock mutex: %d\n", 
            __FILE__, __LINE__, __func__, errno);
                
        exit(EXIT_FAILURE);
    }
}

static const char *slog_get_indent(slog_flag_t eFlag)
{
    slog_config_t *pCfg = &g_slog.config;
    if (!pCfg->nIndent) return SLOG_EMPTY;

    switch (eFlag)
    {
        case SLOG_NOTAG:
            return SLOG_INDENT;
        case SLOG_NOTE:
        case SLOG_INFO:
        case SLOG_WARN:
             return SLOG_SPACE;
        case SLOG_DEBUG:
        case SLOG_TRACE:
        case SLOG_FATAL:
        case SLOG_ERROR:
        default: break;
    }

    return SLOG_EMPTY;
}

static const char* slog_get_tag(slog_flag_t eFlag)
{
    switch (eFlag)
    {
        case SLOG_NOTE: return "note";
        case SLOG_INFO: return "info";
        case SLOG_WARN: return "warn";
        case SLOG_DEBUG: return "debug";
        case SLOG_ERROR: return "error";
        case SLOG_TRACE: return "trace";
        case SLOG_FATAL: return "fatal";
        default: break;
    }

    return NULL;
}

static const char* slog_get_color(slog_flag_t eFlag)
{
    switch (eFlag)
    {
        case SLOG_NOTAG:
        case SLOG_NOTE: return SLOG_EMPTY;
        case SLOG_INFO: return SLOG_COLOR_GREEN;
        case SLOG_WARN: return SLOG_COLOR_YELLOW;
        case SLOG_DEBUG: return SLOG_COLOR_BLUE;
        case SLOG_ERROR: return SLOG_COLOR_RED;
        case SLOG_TRACE: return SLOG_COLOR_CYAN;
        case SLOG_FATAL: return SLOG_COLOR_MAGENTA;
        default: break;
    }

    return SLOG_EMPTY;
}

static void slog_close_file(slog_file_t *pFile)
{
    if (pFile->pHandle != NULL)
    {
        fclose(pFile->pHandle);
        pFile->pHandle = NULL;
    }
}

static uint8_t slog_open_file(slog_file_t *pFile, const slog_config_t *pCfg, const slog_date_t *pDate)
{
    slog_close_file(pFile);

    char sFilePath[SLOG_PATH_MAX + SLOG_NAME_MAX + SLOG_DATE_MAX];
    snprintf(sFilePath, sizeof(sFilePath), "%s/%s-%04d-%02d-%02d.log",
        pCfg->sFilePath, pCfg->sFileName, pDate->nYear, pDate->nMonth, pDate->nDay);

#ifdef _WIN32
    if (fopen_s(&pFile->pHandle, sFilePath, "a")) pFile->pHandle = NULL;
#else
    pFile->pHandle = fopen(sFilePath, "a");
#endif

    if (pFile->pHandle == NULL)
    {
        printf("<%s:%d> %s: [ERROR] Failed to open file: %s (%s)\n",
            __FILE__, __LINE__, __func__, sFilePath, strerror(errno));

        return 0;
    }

    pFile->nCurrDay = pDate->nDay;
    return 1;
}

uint16_t slog_get_usec()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) return 0;
    return (uint16_t)(tv.tv_usec / 1000);
}

void slog_get_date(slog_date_t *pDate)
{
    struct tm timeinfo;
    time_t rawtime = time(NULL);
#ifdef WIN32
    localtime_s(&timeinfo, &rawtime);
#else
    localtime_r(&rawtime, &timeinfo);
#endif

    pDate->nYear = timeinfo.tm_year + 1900;
    pDate->nMonth = timeinfo.tm_mon + 1;
    pDate->nDay = timeinfo.tm_mday;
    pDate->nHour = timeinfo.tm_hour;
    pDate->nMin = timeinfo.tm_min;
    pDate->nSec = timeinfo.tm_sec;
    pDate->nUsec = slog_get_usec();
}

static size_t slog_get_tid()
{
#ifdef __linux__
    return syscall(__NR_gettid);
#elif _WIN32
    return (size_t)GetCurrentThreadId();
#else
    return (size_t)pthread_self();
#endif
}

static void slog_create_tag(char *pOut, size_t nSize, slog_flag_t eFlag, const char *pColor)
{
    slog_config_t *pCfg = &g_slog.config;
    pOut[0] = SLOG_NUL;

    const char *pIndent = slog_get_indent(eFlag);
    const char *pTag = slog_get_tag(eFlag);

    if (pTag == NULL)
    {
        snprintf(pOut, nSize, "%s", pIndent);
        return;
    }

    if (pCfg->eColorFormat != SLOG_COLORING_TAG) snprintf(pOut, nSize, "<%s>%s", pTag, pIndent);
    else snprintf(pOut, nSize, "%s<%s>%s%s", pColor, pTag, SLOG_COLOR_RESET, pIndent);
}

static void slog_create_tid(char *pOut, int nSize, uint8_t nTraceTid)
{
    if (!nTraceTid) pOut[0] = SLOG_NUL;
    else snprintf(pOut, nSize, "(%zu) ", slog_get_tid());
}

static void slog_display_message(const slog_context_t *pCtx, const char *pInfo, int nInfoLen, const char *pInput)
{
    slog_config_t *pCfg = &g_slog.config;
    slog_file_t *pFile = &g_slog.logFile;
    int nCbVal = 1;

    uint8_t nFullColor = pCfg->eColorFormat == SLOG_COLORING_FULL ? 1 : 0;
    const char *pSeparator = nInfoLen > 0 ? pCfg->sSeparator : SLOG_EMPTY;
    const char *pNewLine = pCtx->nNewLine ? SLOG_NEWLINE : SLOG_EMPTY;
    const char *pMessage = pInput != NULL ? pInput : SLOG_EMPTY;
    const char *pReset = nFullColor ? SLOG_COLOR_RESET : SLOG_EMPTY;

    if (pCfg->logCallback != NULL)
    {
        size_t nLength = 0;
        char *pLog = NULL;

        nLength += asprintf(&pLog, "%s%s%s%s%s", pInfo,
            pSeparator, pMessage, pReset, pNewLine);

        if (pLog != NULL)
        {
            nCbVal = pCfg->logCallback (
                pLog,
                nLength,
                pCtx->eFlag,
                pCfg->pCallbackCtx
            );

            free(pLog);
        }
    }

    if (pCfg->nToScreen && nCbVal > 0)
    {
        printf("%s%s%s%s%s", pInfo, pSeparator, pMessage, pReset, pNewLine);
        if (pCfg->nFlush) fflush(stdout);
    }

    if (!pCfg->nToFile || nCbVal < 0) return;
    const slog_date_t *pDate = &pCtx->date;

    if (pFile->nCurrDay != pDate->nDay && pCfg->nRotate) slog_close_file(pFile);
    if (pFile->pHandle == NULL && !slog_open_file(pFile, pCfg, pDate)) return;

    fprintf(pFile->pHandle, "%s%s%s%s%s", pInfo,
        pSeparator, pMessage, pReset, pNewLine);

    if (pCfg->nFlush) fflush(pFile->pHandle);
    if (!pCfg->nKeepOpen) slog_close_file(pFile);
}

static int slog_create_info(const slog_context_t *pCtx, char* pOut, size_t nSize)
{
    slog_config_t *pCfg = &g_slog.config;
    const slog_date_t *pDate = &pCtx->date;

    char sDate[SLOG_DATE_MAX + SLOG_NAME_MAX];
    sDate[0] = SLOG_NUL;

    if (pCfg->eDateControl == SLOG_TIME_ONLY)
    {
        snprintf(sDate, sizeof(sDate), "%02d:%02d:%02d.%03d ",
            pDate->nHour, pDate->nMin, pDate->nSec, pDate->nUsec);
    }
    else if (pCfg->eDateControl == SLOG_DATE_FULL)
    {
        snprintf(sDate, sizeof(sDate), "%04d.%02d.%02d-%02d:%02d:%02d.%03d ",
            pDate->nYear, pDate->nMonth, pDate->nDay, pDate->nHour,
            pDate->nMin, pDate->nSec, pDate->nUsec);
    }

    char sTid[SLOG_TAG_MAX], sTag[SLOG_TAG_MAX];
    uint8_t nFullColor = pCfg->eColorFormat == SLOG_COLORING_FULL ? 1 : 0;

    const char *pColorCode = slog_get_color(pCtx->eFlag);
    const char *pColor = nFullColor ? pColorCode : SLOG_EMPTY;

    slog_create_tid(sTid, sizeof(sTid), pCfg->nTraceTid);
    slog_create_tag(sTag, sizeof(sTag), pCtx->eFlag, pColorCode);
    return snprintf(pOut, nSize, "%s%s%s%s", pColor, sTid, sDate, sTag);
}

static void slog_display_heap(const slog_context_t *pCtx, va_list args)
{
    size_t nBytes = 0;
    char *pMessage = NULL;
    char sLogInfo[SLOG_INFO_MAX];

    nBytes += vasprintf(&pMessage, pCtx->pFormat, args);
    va_end(args);

    if (pMessage == NULL)
    {
        printf("<%s:%d> %s<error>%s %s: Can not allocate memory for input: errno(%d)\n", 
            __FILE__, __LINE__, SLOG_COLOR_RED, SLOG_COLOR_RESET, __func__, errno);

        return;
    }

    int nLength = slog_create_info(pCtx, sLogInfo, sizeof(sLogInfo));
    slog_display_message(pCtx, sLogInfo, nLength, pMessage);
    if (pMessage != NULL) free(pMessage);
}

static void slog_display_stack(const slog_context_t *pCtx, va_list args)
{
    char sMessage[SLOG_MESSAGE_MAX];
    char sLogInfo[SLOG_INFO_MAX];

    vsnprintf(sMessage, sizeof(sMessage), pCtx->pFormat, args);
    int nLength = slog_create_info(pCtx, sLogInfo, sizeof(sLogInfo));
    slog_display_message(pCtx, sLogInfo, nLength, sMessage);
}

void slog_display(slog_flag_t eFlag, uint8_t nNewLine, char *pFormat, ...)
{
    slog_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    if ((SLOG_FLAGS_CHECK(g_slog.config.nFlags, eFlag)) &&
        (g_slog.config.logCallback ||
         g_slog.config.nToScreen ||
         g_slog.config.nToFile))
    {
        slog_context_t ctx;
        slog_get_date(&ctx.date);

        ctx.eFlag = eFlag;
        ctx.pFormat = pFormat;
        ctx.nNewLine = nNewLine;

        void(*slog_display_args)(const slog_context_t *pCtx, va_list args);
        slog_display_args = pCfg->nUseHeap ? slog_display_heap : slog_display_stack;

        va_list args;
        va_start(args, pFormat);
        slog_display_args(&ctx, args);
        va_end(args);
    }

    slog_unlock(&g_slog);
}

const char* slog_version(uint8_t nShort)
{
    if (nShort)
    {
        if (!g_nHaveSlogVerShort)
        {
            snprintf(g_slogVerShort, sizeof(g_slogVerShort), "%d.%d.%d",
                SLOG_VERSION_MAJOR, SLOG_VERSION_MINOR, SLOG_BUILD_NUMBER);

            g_nHaveSlogVerShort = 1;
        }

        return g_slogVerShort;
    }

    if (!g_nHaveSlogVerLong)
    {
        snprintf(g_slogVerLong, sizeof(g_slogVerLong), "%d.%d build %d (%s)",
        SLOG_VERSION_MAJOR, SLOG_VERSION_MINOR, SLOG_BUILD_NUMBER, __DATE__);

        g_nHaveSlogVerLong = 1;
    }

    return g_slogVerLong;
}

void slog_config_get(slog_config_t *pCfg)
{
    slog_lock(&g_slog);
    *pCfg = g_slog.config;
    slog_unlock(&g_slog);
}

void slog_config_set(slog_config_t *pCfg)
{
    slog_lock(&g_slog);
    slog_config_t *pOldCfg = &g_slog.config;
    slog_file_t *pFile = &g_slog.logFile;

    if (!pCfg->nToFile ||
        strncmp(pOldCfg->sFilePath, pCfg->sFilePath, sizeof(pOldCfg->sFilePath)) ||
        strncmp(pOldCfg->sFileName, pCfg->sFileName, sizeof(pOldCfg->sFileName)))
            slog_close_file(pFile); /* Log function will open it again if required */

    g_slog.config = *pCfg;
    slog_unlock(&g_slog);
}

void slog_enable(slog_flag_t eFlag)
{
    slog_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    if (eFlag == SLOG_FLAGS_ALL) pCfg->nFlags = SLOG_FLAGS_ALL;
    else if (!SLOG_FLAGS_CHECK(pCfg->nFlags, eFlag)) pCfg->nFlags |= eFlag;

    slog_unlock(&g_slog);
}

void slog_disable(slog_flag_t eFlag)
{
    slog_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    if (eFlag == SLOG_FLAGS_ALL) pCfg->nFlags = 0;
    else if (SLOG_FLAGS_CHECK(pCfg->nFlags, eFlag)) pCfg->nFlags &= ~eFlag;

    slog_unlock(&g_slog);
}

void slog_separator_set(const char *pFormat, ...)
{
    slog_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    va_list args;
    va_start(args, pFormat);

    if (vsnprintf(pCfg->sSeparator, sizeof(pCfg->sSeparator), pFormat, args) <= 0)
    {
        pCfg->sSeparator[0] = ' ';
        pCfg->sSeparator[1] = '\0';
    }

    va_end(args);
    slog_unlock(&g_slog);
}

void slog_indent(uint8_t nEnable)
{
    slog_lock(&g_slog);
    g_slog.config.nIndent = nEnable;
    slog_unlock(&g_slog);
}

void slog_callback_set(slog_cb_t callback, void *pContext)
{
    slog_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;
    pCfg->pCallbackCtx = pContext;
    pCfg->logCallback = callback;
    slog_unlock(&g_slog);
}

void slog_init(const char* pName, uint16_t nFlags, uint8_t nTdSafe)
{
    slog_config_t *pCfg = &g_slog.config;
    slog_file_t *pFile = &g_slog.logFile;

    /* Set up default values */
    pCfg->eColorFormat = SLOG_COLORING_TAG;
    pCfg->eDateControl = SLOG_TIME_ONLY;
    pCfg->pCallbackCtx = NULL;
    pCfg->logCallback = NULL;
    pCfg->sSeparator[0] = ' ';
    pCfg->sSeparator[1] = '\0';
    pCfg->sFilePath[0] = '.';
    pCfg->sFilePath[1] = '\0';
    pCfg->nKeepOpen = 0;
    pCfg->nTraceTid = 0;
    pCfg->nToScreen = 1;
    pCfg->nUseHeap = 0;
    pCfg->nToFile = 0;
    pCfg->nIndent = 0;
    pCfg->nRotate = 1;
    pCfg->nFlush = 0;
    pCfg->nFlags = nFlags;

    const char *pFileName = (pName != NULL) ? pName : SLOG_NAME_DEFAULT;
    snprintf(pCfg->sFileName, sizeof(pCfg->sFileName), "%s", pFileName);

    pFile->pHandle = NULL;
    pFile->nCurrDay = 0;

#ifdef _WIN32
    /* Enable color support */
    DWORD dwMode = 0;
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hOutput, &dwMode);
    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOutput, dwMode);
#endif

    /* Initialize mutex */
    g_slog.nTdSafe = nTdSafe;
    slog_sync_init(&g_slog);
}

void slog_destroy()
{
    slog_lock(&g_slog);

    slog_close_file(&g_slog.logFile);
    memset(&g_slog.config, 0, sizeof(g_slog.config));

    g_slog.config.pCallbackCtx = NULL;
    g_slog.config.logCallback = NULL;

    slog_unlock(&g_slog);

    if (g_slog.nTdSafe)
    {
        pthread_mutex_destroy(&g_slog.mutex);
        g_slog.nTdSafe = 0;
    }
}