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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "slog.h"

#if !defined(__APPLE__) && !defined(DARWIN) && !defined(WIN32)
#include <syscall.h>
#endif
#include <sys/time.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifndef PTHREAD_MUTEX_RECURSIVE 
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

typedef struct slog {
    unsigned int nTdSafe:1;
    pthread_mutex_t mutex;
    slog_config_t config;
} slog_t;

typedef struct XLogCtx {
    const char *pFormat;
    slog_flag_t eFlag;
    slog_date_t date;
    uint8_t nFullColor;
    uint8_t nNewLine;
} slog_context_t;

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
            __FILE__, __LINE__, __FUNCTION__, errno);

        exit(EXIT_FAILURE);
    }
}

static void slog_lock(slog_t *pSlog)
{
    if (pSlog->nTdSafe && pthread_mutex_lock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not lock mutex: %d\n", 
            __FILE__, __LINE__, __FUNCTION__, errno);

        exit(EXIT_FAILURE);
    }
}

static void slog_unlock(slog_t *pSlog)
{
    if (pSlog->nTdSafe && pthread_mutex_unlock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not unlock mutex: %d\n", 
            __FILE__, __LINE__, __FUNCTION__, errno);
                
        exit(EXIT_FAILURE);
    }
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
        case SLOG_NOTE: return SLOG_COLOR_NORMAL;
        case SLOG_INFO: return SLOG_COLOR_GREEN;
        case SLOG_WARN: return SLOG_COLOR_YELLOW;
        case SLOG_DEBUG: return SLOG_COLOR_BLUE;
        case SLOG_ERROR: return SLOG_COLOR_RED;
        case SLOG_TRACE: return SLOG_COLOR_CYAN;
        case SLOG_FATAL: return SLOG_COLOR_MAGENTA;
        default: break;
    }

    return SLOG_COLOR_NORMAL;
}

uint8_t slog_get_usec()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) return 0;
    return (uint8_t)(tv.tv_usec / 10000);
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

static uint32_t slog_get_tid()
{
#if defined(__APPLE__) || defined(DARWIN) || defined(WIN32)
    return (uint32_t)pthread_self();
#else
    return syscall(__NR_gettid);
#endif
}

static void slog_create_tag(char *pOut, size_t nSize, slog_flag_t eFlag, const char *pColor)
{
    slog_config_t *pCfg = &g_slog.config;
    pOut[0] = SLOG_NUL;

    const char *pTag = slog_get_tag(eFlag);
    if (pTag == NULL) return;

    if (pCfg->eColorFormat != SLOG_COLORING_TAG) snprintf(pOut, nSize, "<%s> ", pTag);
    else snprintf(pOut, nSize, "%s<%s>%s ", pColor, pTag, SLOG_COLOR_RESET);
}

static void slog_create_tid(char *pOut, int nSize, uint8_t nTraceTid)
{
    if (!nTraceTid) pOut[0] = SLOG_NUL;
    else snprintf(pOut, nSize, "(%u) ", slog_get_tid());
}

static void slog_display_message(const slog_context_t *pCtx, const char *pInfo, const char *pInput)
{
    const char *pReset = pCtx->nFullColor ? SLOG_COLOR_RESET : SLOG_EMPTY;
    const char *pNewLine = pCtx->nNewLine ? SLOG_NEWLINE : SLOG_EMPTY;
    const char *pMessage = pInput != NULL ? pInput : SLOG_EMPTY;

    slog_config_t *pCfg = &g_slog.config;
    int nCbVal = 1;

    if (pCfg->logCallback != NULL)
    {
        size_t nLength = 0;
        char *pLog = NULL;

        nLength += asprintf(&pLog, "%s%s%s%s", pInfo, pMessage, pReset, pNewLine);
        if (pLog != NULL)
        {
            nCbVal = pCfg->logCallback(pLog, nLength, pCtx->eFlag, pCfg->pCallbackCtx);
            free(pLog);
        }
    }

    if (pCfg->nToScreen && nCbVal > 0)
    {
        printf("%s%s%s%s", pInfo, pMessage, pReset, pNewLine);
        if (pCfg->nFlush) fflush(stdout);
    }

    if (!pCfg->nToFile || nCbVal < 0) return;
    const slog_date_t *pDate = &pCtx->date;

    char sFilePath[SLOG_PATH_MAX + SLOG_NAME_MAX + SLOG_DATE_MAX];
    snprintf(sFilePath, sizeof(sFilePath), "%s/%s-%04d-%02d-%02d.log", 
        pCfg->sFilePath, pCfg->sFileName, pDate->nYear, pDate->nMonth, pDate->nDay);

    FILE *pFile = fopen(sFilePath, "a");
    if (pFile == NULL) return;

    fprintf(pFile, "%s%s%s%s", pInfo, pMessage, pReset, pNewLine);
    fclose(pFile);
}

static void slog_create_info(const slog_context_t *pCtx, char* pOut, size_t nSize)
{
    slog_config_t *pCfg = &g_slog.config;
    const slog_date_t *pDate = &pCtx->date;

    char sDate[SLOG_DATE_MAX + SLOG_NAME_MAX];
    sDate[0] = SLOG_NUL;

    if (pCfg->eDateControl == SLOG_TIME_ONLY)
    {
        snprintf(sDate, sizeof(sDate),
            "%02d:%02d:%02d.%03d%s",
            pDate->nHour,pDate->nMin,
            pDate->nSec, pDate->nUsec,
            pCfg->sSeparator);
    }
    else if (pCfg->eDateControl == SLOG_DATE_FULL)
    {
        snprintf(sDate, sizeof(sDate),
            "%04d.%02d.%02d-%02d:%02d:%02d.%03d%s",
            pDate->nYear, pDate->nMonth,
            pDate->nDay, pDate->nHour,
            pDate->nMin, pDate->nSec,
            pDate->nUsec, pCfg->sSeparator);
    }

    char sTid[SLOG_TAG_MAX], sTag[SLOG_TAG_MAX];
    const char *pColorCode = slog_get_color(pCtx->eFlag);
    const char *pColor = pCtx->nFullColor ? pColorCode : SLOG_EMPTY;

    slog_create_tid(sTid, sizeof(sTid), pCfg->nTraceTid);
    slog_create_tag(sTag, sizeof(sTag), pCtx->eFlag, pColorCode);
    snprintf(pOut, nSize, "%s%s%s%s", pColor, sTid, sDate, sTag); 
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
            __FILE__, __LINE__, SLOG_COLOR_RED, SLOG_COLOR_RESET, __FUNCTION__, errno);

        return;
    }

    slog_create_info(pCtx, sLogInfo, sizeof(sLogInfo));
    slog_display_message(pCtx, sLogInfo, pMessage);
    if (pMessage != NULL) free(pMessage);
}

static void slog_display_stack(const slog_context_t *pCtx, va_list args)
{
    char sMessage[SLOG_MESSAGE_MAX];
    char sLogInfo[SLOG_INFO_MAX];

    vsnprintf(sMessage, sizeof(sMessage), pCtx->pFormat, args);
    slog_create_info(pCtx, sLogInfo, sizeof(sLogInfo));
    slog_display_message(pCtx, sLogInfo, sMessage);
}

void slog_display(slog_flag_t eFlag, uint8_t nNewLine, const char *pFormat, ...)
{
    slog_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    if ((SLOG_FLAGS_CHECK(g_slog.config.nFlags, eFlag)) &&
       (g_slog.config.nToScreen || g_slog.config.nToFile))
    {
        slog_context_t ctx;
        slog_get_date(&ctx.date);

        ctx.eFlag = eFlag;
        ctx.pFormat = pFormat;
        ctx.nNewLine = nNewLine;
        ctx.nFullColor = pCfg->eColorFormat == SLOG_COLORING_FULL ? 1 : 0;

        void(*slog_display_args)(const slog_context_t *pCtx, va_list args);
        slog_display_args = pCfg->nUseHeap ? slog_display_heap : slog_display_stack;

        va_list args;
        va_start(args, pFormat);
        slog_display_args(&ctx, args);
        va_end(args);
    }

    slog_unlock(&g_slog);
}

size_t slog_version(char *pDest, size_t nSize, uint8_t nMin)
{
    size_t nLength = 0;

    /* Version short */
    if (nMin) nLength = snprintf(pDest, nSize, "%d.%d.%d", 
        SLOG_VERSION_MAJOR, SLOG_VERSION_MINOR, SLOG_BUILD_NUM);

    /* Version long */
    else nLength = snprintf(pDest, nSize, "%d.%d build %d (%s)", 
        SLOG_VERSION_MAJOR, SLOG_VERSION_MINOR, SLOG_BUILD_NUM, __DATE__);

    return nLength;
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
    g_slog.config = *pCfg;
    slog_unlock(&g_slog);
}

void slog_enable(slog_flag_t eFlag)
{
    slog_lock(&g_slog);

    if (!SLOG_FLAGS_CHECK(g_slog.config.nFlags, eFlag))
        g_slog.config.nFlags |= eFlag;

    slog_unlock(&g_slog);
}

void slog_disable(slog_flag_t eFlag)
{
    slog_lock(&g_slog);

    if (SLOG_FLAGS_CHECK(g_slog.config.nFlags, eFlag))
        g_slog.config.nFlags &= ~eFlag;

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
    /* Set up default values */
    slog_config_t *pCfg = &g_slog.config;
    pCfg->eColorFormat = SLOG_COLORING_TAG;
    pCfg->eDateControl = SLOG_TIME_ONLY;
    pCfg->pCallbackCtx = NULL;
    pCfg->logCallback = NULL;
    pCfg->sSeparator[0] = ' ';
    pCfg->sSeparator[1] = '\0';
    pCfg->sFilePath[0] = '.';
    pCfg->sFilePath[1] = '\0';
    pCfg->nTraceTid = 0;
    pCfg->nToScreen = 1;
    pCfg->nUseHeap = 0;
    pCfg->nToFile = 0;
    pCfg->nFlush = 0;
    pCfg->nFlags = nFlags;

    const char *pFileName = (pName != NULL) ? pName : SLOG_NAME_DEFAULT;
    snprintf(pCfg->sFileName, sizeof(pCfg->sFileName), "%s", pFileName);

#ifdef WIN32
    // Enable color support
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
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
    g_slog.config.pCallbackCtx = NULL;
    g_slog.config.logCallback = NULL;

    if (g_slog.nTdSafe)
    {
        pthread_mutex_destroy(&g_slog.mutex);
        g_slog.nTdSafe = 0;
    }
}