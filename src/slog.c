/*
 * The MIT License (MIT)
 *
 *  Copyleft (C) 2015-2025  Sandro Kalatozishvili (s.kalatoz@gmail.com)
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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "slog.h"

#ifdef __linux__
#include <syscall.h>
#endif

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#else
#include <windows.h>
#include <share.h>
#endif

#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

#define SLOG_FILE_PATH_MAX (SLOG_PATH_MAX + SLOG_NAME_MAX + SLOG_DATE_MAX)
#define SLOG_ASSERT_RET(x) if (!(x)) return

#define SLOG_STRFY_RAW(x) #x
#define SLOG_STRFY(x) SLOG_STRFY_RAW(x)

typedef struct slog_file {
    char sFilePath[SLOG_FILE_PATH_MAX];
    uint16_t nCurrYear;
    uint8_t nCurrMonth;
    uint8_t nCurrDay;
    FILE *pHandle;
} slog_file_t;

typedef struct slog {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
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

static const char g_slogVerShort[] =
    SLOG_STRFY(SLOG_VERSION_MAJOR) "."
    SLOG_STRFY(SLOG_VERSION_MINOR) "."
    SLOG_STRFY(SLOG_BUILD_NUMBER);

static const char g_slogVerLong[] =
    SLOG_STRFY(SLOG_VERSION_MAJOR) "."
    SLOG_STRFY(SLOG_VERSION_MINOR) " build "
    SLOG_STRFY(SLOG_BUILD_NUMBER)
    " (" SLOG_RELEASE_DATE ")";

static volatile int g_nSlogInit = 0;
static slog_t g_slog;

static void slog_sync_init(slog_t *pSlog)
{
    SLOG_ASSERT_RET(pSlog->nTdSafe);

#ifndef _WIN32
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
#else
    InitializeCriticalSection(&pSlog->mutex);
#endif
}

static void slog_sync_destroy(slog_t *pSlog)
{
    SLOG_ASSERT_RET(pSlog->nTdSafe);

#ifndef _WIN32
    pthread_mutex_destroy(&pSlog->mutex);
#else
    DeleteCriticalSection(&pSlog->mutex);
#endif

    pSlog->nTdSafe = 0;
}

static void slog_sync_lock(slog_t *pSlog)
{
    SLOG_ASSERT_RET(pSlog->nTdSafe);

#ifndef _WIN32
    if (pthread_mutex_lock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not lock mutex: %d\n",
            __FILE__, __LINE__, __func__, errno);

        exit(EXIT_FAILURE);
    }
#else
    EnterCriticalSection(&pSlog->mutex);
#endif
}

static void slog_sync_unlock(slog_t *pSlog)
{
    SLOG_ASSERT_RET(pSlog->nTdSafe);

#ifndef _WIN32
    if (pthread_mutex_unlock(&pSlog->mutex))
    {
        printf("<%s:%d> %s: [ERROR] Can not unlock mutex: %d\n",
            __FILE__, __LINE__, __func__, errno);

        exit(EXIT_FAILURE);
    }
#else
    LeaveCriticalSection(&pSlog->mutex);
#endif
}

#ifdef _WIN32
int slog_vasprintf(char **ppStr, const char *pFmt, va_list args)
{
    va_list locArgs;
#ifdef va_copy
    va_copy(locArgs, args);
#else
    memcpy(&locArgs, &args, sizeof(va_list));
#endif

    int nLength = vsnprintf(NULL, 0, pFmt, locArgs);
    if (nLength < 0) return -1;

    *ppStr = (char *)malloc(nLength + 1);
    if (*ppStr == NULL) return -1;

    int nResult = vsnprintf(*ppStr, nLength + 1, pFmt, args);
    if (nResult <= 0)
    {
        free(*ppStr);
        *ppStr = NULL;
        return -1;
    }

    int nLen = nResult < nLength ? nResult : nLength;
    char *pFinal = *ppStr;
    pFinal[nLen] = '\0';

    return nLen;
}

int slog_asprintf(char **ppStr, const char *pFmt, ...)
{
    va_list args;
    va_start(args, pFmt);
    int nResult = slog_vasprintf(ppStr, pFmt, args);
    va_end(args);
    return nResult;
}
#endif

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

#ifdef _WIN32
void slog_get_date(slog_date_t *pDate)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    pDate->nYear = (uint16_t)st.wYear;
    pDate->nMonth = (uint8_t)st.wMonth;
    pDate->nDay = (uint8_t)st.wDay;
    pDate->nHour = (uint8_t)st.wHour;
    pDate->nMin = (uint8_t)st.wMinute;
    pDate->nSec = (uint8_t)st.wSecond;
    pDate->nUsec = (uint16_t)st.wMilliseconds;
}
#else
void slog_get_date(slog_date_t *pDate)
{
    struct timeval tv;
    struct tm tm_info;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);

    pDate->nYear = tm_info.tm_year + 1900;
    pDate->nMonth = tm_info.tm_mon + 1;
    pDate->nDay = tm_info.tm_mday;
    pDate->nHour = tm_info.tm_hour;
    pDate->nMin = tm_info.tm_min;
    pDate->nSec = tm_info.tm_sec;
    pDate->nUsec = (uint16_t)(tv.tv_usec / 1000);
}
#endif

uint16_t slog_get_usec()
{
    slog_date_t date;
    slog_get_date(&date);
    return date.nUsec;
}

static void slog_date_from_epoch(slog_date_t *pDate, time_t nTime)
{
    struct tm tm_info;

#ifdef _WIN32
    localtime_s(&tm_info, &nTime);
#else
    localtime_r(&nTime, &tm_info);
#endif

    pDate->nYear = (uint16_t)(tm_info.tm_year + 1900);
    pDate->nMonth = (uint8_t)(tm_info.tm_mon + 1);
    pDate->nDay = (uint8_t)tm_info.tm_mday;
    pDate->nHour = (uint8_t)tm_info.tm_hour;
    pDate->nMin = (uint8_t)tm_info.tm_min;
    pDate->nSec = (uint8_t)tm_info.tm_sec;
    pDate->nUsec = 0;
}

static void slog_close_file(slog_file_t *pFile)
{
    if (pFile->pHandle != NULL)
    {
        fclose(pFile->pHandle);
        pFile->pHandle = NULL;
    }
}

/* Move the active log file into the dated archive of the day it belongs to */
static void slog_rotate_file(slog_file_t *pFile, const slog_config_t *pCfg)
{
    struct stat statBuf;
    slog_close_file(pFile);

    memset(&statBuf, 0, sizeof(statBuf));
    if (stat(pFile->sFilePath, &statBuf) < 0) return;

    char sRotatedPath[SLOG_FILE_PATH_MAX];
    int nLength = snprintf(sRotatedPath, sizeof(sRotatedPath), "%s/%s-%04d-%02d-%02d.log",
        pCfg->sFilePath, pCfg->sFileName, pFile->nCurrYear, pFile->nCurrMonth, pFile->nCurrDay);

    if (nLength <= 0) return;

#ifdef _WIN32
    /* Unlike POSIX, rename() on Windows fails if the destination already exists */
    remove(sRotatedPath);
#endif

    rename(pFile->sFilePath, sRotatedPath);
}

static uint8_t slog_open_file(slog_file_t *pFile, const slog_config_t *pCfg, const slog_date_t *pDate)
{
    struct stat statBuf;
    slog_close_file(pFile);

    if (pFile->sFilePath[0] == SLOG_NUL)
    {
        snprintf(pFile->sFilePath, sizeof(pFile->sFilePath), "%s/%s.log",
            pCfg->sFilePath, pCfg->sFileName);
    }

    /* Check if the existing log file belongs to a different day and archive it */
    memset(&statBuf, 0, sizeof(statBuf));
    if (pCfg->nRotate && stat(pFile->sFilePath, &statBuf) == 0)
    {
        slog_date_t modDate;
        slog_date_from_epoch(&modDate, (time_t)statBuf.st_mtime);

        if (modDate.nDay != pDate->nDay ||
            modDate.nMonth != pDate->nMonth ||
            modDate.nYear != pDate->nYear)
        {
            pFile->nCurrYear = modDate.nYear;
            pFile->nCurrMonth = modDate.nMonth;
            pFile->nCurrDay = modDate.nDay;
            slog_rotate_file(pFile, pCfg);
        }
    }

#ifdef _WIN32
    /* Keep the file readable for other processes while the handle is open */
    pFile->pHandle = _fsopen(pFile->sFilePath, "a", _SH_DENYNO);
#else
    pFile->pHandle = fopen(pFile->sFilePath, "a");
#endif

    if (pFile->pHandle == NULL)
    {
#ifdef _WIN32
        char sError[SLOG_INFO_MAX];
        strerror_s(sError, sizeof(sError), errno);
        char *pError = sError;
#else
        char *pError = strerror(errno);
#endif

        printf("<%s:%d> %s: [ERROR] Failed to open file: %s (%s)\n",
            __FILE__, __LINE__, __func__, pFile->sFilePath, pError);

        return 0;
    }

    pFile->nCurrYear = pDate->nYear;
    pFile->nCurrMonth = pDate->nMonth;
    pFile->nCurrDay = pDate->nDay;

    return 1;
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
        int nLength = 0;
        char *pLog = NULL;

#ifdef _WIN32
        nLength = slog_asprintf(&pLog, "%s%s%s%s%s", pInfo,
            pSeparator, pMessage, pReset, pNewLine);
#else
        nLength = asprintf(&pLog, "%s%s%s%s%s", pInfo,
            pSeparator, pMessage, pReset, pNewLine);
#endif

        if (pLog != NULL && nLength > 0)
        {
            nCbVal = pCfg->logCallback (
                pLog,
                (size_t)nLength,
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

    if (pCfg->nRotate &&
       (pFile->nCurrDay != pDate->nDay ||
        pFile->nCurrMonth != pDate->nMonth ||
        pFile->nCurrYear != pDate->nYear))
    {
        /* Zero day means we did not open the log file yet, nothing to archive */
        if (!pFile->nCurrDay) slog_close_file(pFile);
        else slog_rotate_file(pFile, pCfg);
    }

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
    int nBytes = 0;
    char *pMessage = NULL;
    char sLogInfo[SLOG_INFO_MAX];

#ifdef _WIN32
    nBytes = slog_vasprintf(&pMessage, pCtx->pFormat, args);
#else
    nBytes = vasprintf(&pMessage, pCtx->pFormat, args);
#endif

    /* Note: args is closed by the caller, closing it twice is undefined */
    (void)nBytes;

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

void slog_display(slog_flag_t eFlag, uint8_t nNewLine, const char *pFormat, ...)
{
    slog_sync_lock(&g_slog);
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

    slog_sync_unlock(&g_slog);
}

uint8_t slog_is_init(void)
{
    return g_nSlogInit ? 1 : 0;
}

const char* slog_version(uint8_t nShort)
{
    return nShort ? g_slogVerShort : g_slogVerLong;
}

void slog_config_get(slog_config_t *pCfg)
{
    slog_sync_lock(&g_slog);
    *pCfg = g_slog.config;
    slog_sync_unlock(&g_slog);
}

void slog_config_set(slog_config_t *pCfg)
{
    slog_sync_lock(&g_slog);
    slog_config_t *pOldCfg = &g_slog.config;
    slog_file_t *pFile = &g_slog.logFile;

    if (!pCfg->nToFile ||
        strncmp(pOldCfg->sFilePath, pCfg->sFilePath, sizeof(pOldCfg->sFilePath)) ||
        strncmp(pOldCfg->sFileName, pCfg->sFileName, sizeof(pOldCfg->sFileName)))
    {
        slog_close_file(pFile); /* Log function will open it again if required */
        pFile->sFilePath[0] = SLOG_NUL;
    }

    g_slog.config = *pCfg;
    slog_sync_unlock(&g_slog);
}

void slog_enable(slog_flag_t eFlag)
{
    slog_sync_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    if (eFlag == SLOG_FLAGS_ALL) pCfg->nFlags = SLOG_FLAGS_ALL;
    else if (!SLOG_FLAGS_CHECK(pCfg->nFlags, eFlag)) pCfg->nFlags |= eFlag;

    slog_sync_unlock(&g_slog);
}

void slog_disable(slog_flag_t eFlag)
{
    slog_sync_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    if (eFlag == SLOG_FLAGS_ALL) pCfg->nFlags = 0;
    else if (SLOG_FLAGS_CHECK(pCfg->nFlags, eFlag)) pCfg->nFlags &= ~eFlag;

    slog_sync_unlock(&g_slog);
}

void slog_separator_set(const char *pFormat, ...)
{
    slog_sync_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;

    va_list args;
    va_start(args, pFormat);

    if (vsnprintf(pCfg->sSeparator, sizeof(pCfg->sSeparator), pFormat, args) <= 0)
    {
        pCfg->sSeparator[0] = ' ';
        pCfg->sSeparator[1] = '\0';
    }

    va_end(args);
    slog_sync_unlock(&g_slog);
}

void slog_callback_set(slog_cb_t callback, void *pContext)
{
    slog_sync_lock(&g_slog);
    slog_config_t *pCfg = &g_slog.config;
    pCfg->pCallbackCtx = pContext;
    pCfg->logCallback = callback;
    slog_sync_unlock(&g_slog);
}

size_t slog_get_full_path(char *pFilePath, size_t nSize)
{
    if (pFilePath == NULL || !nSize) return 0;
    slog_sync_lock(&g_slog);

    slog_file_t *pFile = &g_slog.logFile;
    int nLength = snprintf(pFilePath, nSize, "%s", pFile->sFilePath);

    /* snprintf() returns the length it wanted to write, clamp it to the buffer */
    if (nLength < 0) nLength = 0;
    else if ((size_t)nLength >= nSize) nLength = (int)nSize - 1;
    pFilePath[nLength] = SLOG_NUL;

    slog_sync_unlock(&g_slog);
    return (size_t)nLength;
}

size_t slog_path_set(const char *pPath)
{
    if (pPath == NULL) return 0;
    slog_sync_lock(&g_slog);

    slog_config_t *pCfg = &g_slog.config;
    slog_file_t *pFile = &g_slog.logFile;

    if (strncmp(pCfg->sFilePath, pPath, sizeof(pCfg->sFilePath)))
    {
        slog_close_file(pFile); /* Log function will open it again if required */
        pFile->sFilePath[0] = SLOG_NUL;
    }

    int nLength = snprintf(pCfg->sFilePath, sizeof(pCfg->sFilePath), "%s", pPath);
    if (nLength < 0) nLength = 0;
    else if ((size_t)nLength >= sizeof(pCfg->sFilePath)) nLength = (int)sizeof(pCfg->sFilePath) - 1;

    slog_sync_unlock(&g_slog);
    return (size_t)nLength;
}

size_t slog_name_set(const char *pName)
{
    if (pName == NULL) return 0;
    slog_sync_lock(&g_slog);

    slog_config_t *pCfg = &g_slog.config;
    slog_file_t *pFile = &g_slog.logFile;

    if (strncmp(pCfg->sFileName, pName, sizeof(pCfg->sFileName)))
    {
        slog_close_file(pFile); /* Log function will open it again if required */
        pFile->sFilePath[0] = SLOG_NUL;
    }

    int nLength = snprintf(pCfg->sFileName, sizeof(pCfg->sFileName), "%s", pName);
    if (nLength < 0) nLength = 0;
    else if ((size_t)nLength >= sizeof(pCfg->sFileName)) nLength = (int)sizeof(pCfg->sFileName) - 1;

    slog_sync_unlock(&g_slog);
    return (size_t)nLength;
}

void slog_color_format_set(slog_coloring_t eFmt)
{
    slog_sync_lock(&g_slog);
    g_slog.config.eColorFormat = eFmt;
    slog_sync_unlock(&g_slog);
}

void slog_date_format_set(slog_date_ctrl_t eFmt)
{
    slog_sync_lock(&g_slog);
    g_slog.config.eDateControl = eFmt;
    slog_sync_unlock(&g_slog);
}

void slog_screen_set(uint8_t nEnable)
{
    slog_sync_lock(&g_slog);
    g_slog.config.nToScreen = nEnable;
    slog_sync_unlock(&g_slog);
}

void slog_file_set(uint8_t nEnable)
{
    slog_sync_lock(&g_slog);

    if (!nEnable) slog_close_file(&g_slog.logFile);
    g_slog.config.nToFile = nEnable;

    slog_sync_unlock(&g_slog);
}

void slog_flush_set(uint8_t nEnable)
{
    slog_sync_lock(&g_slog);
    g_slog.config.nFlush = nEnable;
    slog_sync_unlock(&g_slog);
}

void slog_indent_set(uint8_t nEnable)
{
    slog_sync_lock(&g_slog);
    g_slog.config.nIndent = nEnable;
    slog_sync_unlock(&g_slog);
}

void slog_trace_tid_set(uint8_t nEnable)
{
    slog_sync_lock(&g_slog);
    g_slog.config.nTraceTid = nEnable;
    slog_sync_unlock(&g_slog);
}

void slog_use_heap_set(uint8_t nEnable)
{
    slog_sync_lock(&g_slog);
    g_slog.config.nUseHeap = nEnable;
    slog_sync_unlock(&g_slog);
}

void slog_flags_set(uint16_t nFlags)
{
    slog_sync_lock(&g_slog);
    g_slog.config.nFlags = nFlags;
    slog_sync_unlock(&g_slog);
}

uint16_t slog_flags_get(void)
{
    slog_sync_lock(&g_slog);
    uint16_t nFlags = g_slog.config.nFlags;
    slog_sync_unlock(&g_slog);
    return nFlags;
}

void slog_init(const char* pName, uint16_t nFlags, uint8_t nTdSafe)
{
    /* Re-initializing a live mutex is undefined behaviour, so recreate
     * the lock only if this is the first init or the mode has changed */
    if (!g_nSlogInit || g_slog.nTdSafe != nTdSafe)
    {
        if (g_nSlogInit) slog_sync_destroy(&g_slog);
        g_slog.nTdSafe = nTdSafe;
        slog_sync_init(&g_slog);
    }

    slog_sync_lock(&g_slog);

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
    pCfg->nKeepOpen = 1;
    pCfg->nToScreen = 1;
    pCfg->nTraceTid = 0;
    pCfg->nUseHeap = 0;
    pCfg->nToFile = 0;
    pCfg->nIndent = 0;
    pCfg->nRotate = 1;
    pCfg->nFlush = 0;
    pCfg->nFlags = nFlags;

    const char *pFileName = (pName != NULL) ? pName : SLOG_NAME_DEFAULT;
    snprintf(pCfg->sFileName, sizeof(pCfg->sFileName), "%s", pFileName);

    /* Do not leak the handle if we are re-initialized */
    slog_close_file(pFile);

    pFile->sFilePath[0] = SLOG_NUL;
    pFile->nCurrYear = 0;
    pFile->nCurrMonth = 0;
    pFile->nCurrDay = 0;

#ifdef _WIN32
    /* Enable color support */
    DWORD dwMode = 0;
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hOutput, &dwMode);
    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOutput, dwMode);
#endif

    g_nSlogInit = 1;
    slog_sync_unlock(&g_slog);
}

void slog_destroy()
{
    slog_sync_lock(&g_slog);
    slog_close_file(&g_slog.logFile);
    memset(&g_slog.config, 0, sizeof(g_slog.config));

    g_slog.config.pCallbackCtx = NULL;
    g_slog.config.logCallback = NULL;

    g_slog.logFile.sFilePath[0] = SLOG_NUL;
    g_slog.logFile.nCurrYear = 0;
    g_slog.logFile.nCurrMonth = 0;
    g_slog.logFile.nCurrDay = 0;
    g_nSlogInit = 0;

    slog_sync_unlock(&g_slog);
    slog_sync_destroy(&g_slog);
}
