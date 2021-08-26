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

#if !defined(DARWIN) && !defined(WIN32)
#include <syscall.h>
#endif
#include <sys/time.h>

#ifndef PTHREAD_MUTEX_RECURSIVE 
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

typedef struct slog_context_t_ {
    unsigned int nTdSafe:1;
    pthread_mutex_t mutex;
    slog_config_t slogCfg;
    slog_date_t slogDate;
} slog_context_t;

static slog_context_t g_slog;

static void slog_sync_init(slog_context_t *pSlog)
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

static void slog_lock(slog_context_t *pSlog)
{
    if (pSlog->nTdSafe && pthread_mutex_lock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not lock mutex: %d\n", 
            __FILE__, __LINE__, __FUNCTION__, errno);

        exit(EXIT_FAILURE);
    }
}

static void slog_unlock(slog_context_t *pSlog)
{
    if (pSlog->nTdSafe && pthread_mutex_unlock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not unlock mutex: %d\n", 
            __FILE__, __LINE__, __FUNCTION__, errno);
                
        exit(EXIT_FAILURE);
    }
}

static const char* slog_get_tag(SLOG_FLAGS_E eFlag)
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

static const char* slog_get_color(SLOG_FLAGS_E eFlag)
{
    switch (eFlag)
    {
        case SLOG_NOTE: return SLOG_CLR_NORMAL;
        case SLOG_INFO: return SLOG_CLR_GREEN;
        case SLOG_WARN: return SLOG_CLR_YELLOW;
        case SLOG_DEBUG: return SLOG_CLR_BLUE;
        case SLOG_ERROR: return SLOG_CLR_RED;
        case SLOG_TRACE: return SLOG_CLR_CYAN;
        case SLOG_FATAL: return SLOG_CLR_MAGENTA;
        default: break;
    }

    return SLOG_CLR_NORMAL;
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

static void slog_create_tag( char *pOut, size_t nSize, SLOG_FLAGS_E eFlag, const char *pColor)
{
    const char *pTag = slog_get_tag(eFlag);
    if (pTag == NULL) return;

    if (g_slog.slogCfg.eColorFormat != SLOG_COLOR_TAG) snprintf(pOut, nSize, "<%s> ", pTag);
    else snprintf(pOut, nSize, "%s<%s>%s ", pColor, pTag, SLOG_CLR_RESET);
}

static uint32_t slog_get_tid()
{
#if defined(DARWIN) || defined(WIN32)
    return (uint32_t)pthread_self();
#else
    return syscall(__NR_gettid);
#endif
}

static void slog_display_output(char *pStr, uint8_t nNewLine)
{
    if (g_slog.slogCfg.nToScreen)
    {
        printf("%s%s", pStr, nNewLine ? "\n" : "");
        if (g_slog.slogCfg.nFlush) fflush(stdout);
    }

    if (!g_slog.slogCfg.nToFile) return;
    const slog_date_t *pDate = &g_slog.slogDate;

    char sFilePath[SLOG_PATH_MAX + SLOG_NAME_MAX + SLOG_DATE_MAX];
    snprintf(sFilePath, sizeof(sFilePath), "%s/%s-%04d-%02d-%02d.log", 
        g_slog.slogCfg.sFilePath, g_slog.slogCfg.sFileName, 
        pDate->nYear, pDate->nMonth, pDate->nDay);

    FILE *pFile = fopen(sFilePath, "a");
    if (pFile == NULL) return;

    fprintf(pFile, "%s%s", pStr, nNewLine ? "\n" : "");
    fclose(pFile);
}

static void slog_create_output(char* pOut, size_t nSize, const char* pStr, SLOG_FLAGS_E eFlag)
{
    const slog_config_t *pConfig = &g_slog.slogCfg;
    const slog_date_t *pDate = &g_slog.slogDate;
    char sDate[SLOG_DATE_MAX];
    sDate[0] = '\0';

    if (pConfig->eDateControl == SLOG_TIME_ONLY)
    {
        snprintf(sDate, sizeof(sDate), "%02d:%02d:%02d.%02d - ", 
            pDate->nHour, pDate->nMin, pDate->nSec, pDate->nUsec);
    }
    else if (pConfig->eDateControl == SLOG_DATE_FULL)
    {
        snprintf(sDate, sizeof(sDate), "%04d.%02d.%02d-%02d:%02d:%02d.%02d - ", 
            pDate->nYear, pDate->nMonth, pDate->nDay, pDate->nHour, pDate->nMin, 
            pDate->nSec, pDate->nUsec);
    }

    char sTid[SLOG_TAG_MAX];
    char sTag[SLOG_TAG_MAX];
    sTid[0] = sTag[0] = '\0';

    const char *pColor = slog_get_color(eFlag);
    slog_create_tag(sTag, sizeof(sTag), eFlag, pColor);

    if (pConfig->nTraceTid) snprintf(sTid, sizeof(sTid), "(%u) ", slog_get_tid());
    if (pConfig->eColorFormat != SLOG_COLOR_FULL) snprintf(pOut, nSize, "%s%s%s%s", sTid, sDate, sTag, pStr); 
    else snprintf(pOut, nSize, "%s%s%s%s%s%s", pColor, sTid, sDate, sTag, pStr, SLOG_CLR_RESET); 
}

static void slog_display_heap(SLOG_FLAGS_E eFlag, uint8_t nNewLine, const char *pMsg, va_list args)
{
    size_t nBytes = 0;
    char *pInput = NULL;

    nBytes += vasprintf(&pInput, pMsg, args);
    va_end(args);

    if (pInput == NULL)
    {
        printf("<%s:%d> %s<error>%s %s: Can not allocate memory for input: errno(%d)\n", 
            __FILE__, __LINE__, SLOG_CLR_RED, SLOG_CLR_RESET, __FUNCTION__, errno);

        return;
    }

    nBytes += SLOG_DATE_MAX + SLOG_CLR_MAX + (SLOG_TAG_MAX * 2);
    char *pOutput = (char*)malloc(nBytes);

    if (pOutput == NULL)
    {
        printf("[%s:%d] %s<error>%s %s: Can not allocate memory for output: errno(%d)\n", 
            __FILE__, __LINE__, SLOG_CLR_RED, SLOG_CLR_RESET, __FUNCTION__, errno);

        free(pInput);
        return;
    }

    slog_create_output(pOutput, nBytes, pInput, eFlag);
    slog_display_output(pOutput, nNewLine);

    free(pOutput);
    free(pInput);
}

static void slog_display_stack(SLOG_FLAGS_E eFlag, uint8_t nNewLine, const char *pMsg, va_list args)
{
    char sInput[SLOG_MESSAGE_MAX];
    vsnprintf(sInput, sizeof(sInput), pMsg, args);
    va_end(args);

    char sOutput[SLOG_MESSAGE_MAX + SLOG_DATE_MAX + SLOG_CLR_MAX + (SLOG_TAG_MAX * 2)];
    slog_create_output(sOutput, sizeof(sOutput), sInput, eFlag);
    slog_display_output(sOutput, nNewLine);
}

void slog_display(SLOG_FLAGS_E eFlag, uint8_t nNewLine, const char *pMsg, ...)
{
    slog_lock(&g_slog);

    if ((SLOG_FLAGS_CHECK(g_slog.slogCfg.nFlags, eFlag)) &&
       (g_slog.slogCfg.nToScreen || g_slog.slogCfg.nToFile))
    {
        va_list args;
        va_start(args, pMsg);
        slog_get_date(&g_slog.slogDate);

        if (g_slog.slogCfg.nUseHeap) slog_display_heap(eFlag, nNewLine, pMsg, args);
        else slog_display_stack(eFlag, nNewLine, pMsg, args);

    }

    slog_unlock(&g_slog);
}

const char* slog_version(uint8_t nMin)
{
    static char sVersion[SLOG_VERSION_MAX];

    /* Version short */
    if (nMin) snprintf(sVersion, sizeof(sVersion), "%d.%d.%d", 
        SLOG_VERSION_MAJOR, SLOG_VERSION_MINOR, SLOG_BUILD_NUM);

    /* Version long */
    else snprintf(sVersion, sizeof(sVersion), "%d.%d build %d (%s)", 
        SLOG_VERSION_MAJOR, SLOG_VERSION_MINOR, SLOG_BUILD_NUM, __DATE__);

    return sVersion;
}

void slog_config_get(slog_config_t *pCfg)
{
    slog_lock(&g_slog);
    *pCfg = g_slog.slogCfg;
    slog_unlock(&g_slog);
}

void slog_config_set(slog_config_t *pCfg)
{
    slog_lock(&g_slog);
    g_slog.slogCfg = *pCfg;
    slog_unlock(&g_slog);
}

void slog_enable(SLOG_FLAGS_E eFlag)
{
    slog_lock(&g_slog);

    if (!SLOG_FLAGS_CHECK(g_slog.slogCfg.nFlags, eFlag))
        g_slog.slogCfg.nFlags |= eFlag;

    slog_unlock(&g_slog);
}

void slog_disable(SLOG_FLAGS_E eFlag)
{
    slog_lock(&g_slog);

    if (SLOG_FLAGS_CHECK(g_slog.slogCfg.nFlags, eFlag))
        g_slog.slogCfg.nFlags &= ~eFlag;

    slog_unlock(&g_slog);
}

void slog_init(const char* pName, uint16_t nFlags, uint8_t nTdSafe)
{
    /* Set up default values */
    slog_config_t *pConfig = &g_slog.slogCfg;
    pConfig->eColorFormat = SLOG_COLOR_TAG;
    pConfig->eDateControl = SLOG_TIME_ONLY;
    pConfig->sFilePath[0] = '.';
    pConfig->sFilePath[1] = '\0';
    pConfig->nTraceTid = 0;
    pConfig->nToScreen = 1;
    pConfig->nUseHeap = 0;
    pConfig->nToFile = 0;
    pConfig->nFlush = 0;
    pConfig->nFlags = nFlags;

    if (pName != NULL) snprintf(pConfig->sFileName, sizeof(pConfig->sFileName)-1, "%s", pName);
    else snprintf(pConfig->sFileName, sizeof(pConfig->sFileName)-1, "%s", "slog");

    /* Initialize mutex */
    g_slog.nTdSafe = nTdSafe;
    slog_sync_init(&g_slog);
}

void slog_destroy()
{
    if (!g_slog.nTdSafe) return;
    pthread_mutex_destroy(&g_slog.mutex);
    g_slog.nTdSafe = 0;
}