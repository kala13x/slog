/*
 * The MIT License (MIT)
 *
 *  Copyleft (C) 2015  Sun Dro (a.k.a. kala13x)
 *  Copyleft (C) 2017  George G. Gkasdrogkas (a.k.a. GeorgeGkas)
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "slog.h"


/* Max size of string. */
#define MAXMSG 8196


/* Static global variables. */
static SlogFlags slg;
static pthread_mutex_t slog_mutex;


#ifdef DARWIN

/*
 * Bellow we provide an alternative for clock_gettime,
 * which is not implemented in Mac OS X.
 */
static inline int clock_gettime(int clock_id, struct timespec *ts)
{
    struct timeval tv;

    if (clock_id != CLOCK_REALTIME) 
    {
        errno = EINVAL;
        return -1;
    }
    if (gettimeofday(&tv, NULL) < 0) 
    {
        return -1;
    }
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
    return 0;
}

#endif /* DARWIN */

/*
 * Get system date-time.
 */
void slog_get_date(SlogDate *sdate)
{
    time_t rawtime;
    struct tm timeinfo;
    struct timespec now;
    rawtime = time(NULL);
    localtime_r(&rawtime, &timeinfo);

    /* Get System Date-time. */
    sdate->year = timeinfo.tm_year + 1900;
    sdate->mon = timeinfo.tm_mon + 1;
    sdate->day = timeinfo.tm_mday;
    sdate->hour = timeinfo.tm_hour;
    sdate->min = timeinfo.tm_min;
    sdate->sec = timeinfo.tm_sec;

    /* Get micro seconds. */
    clock_gettime(CLOCK_MONOTONIC, &now);
    sdate->usec = now.tv_nsec / 10000000;
}

/*
 * Return program version.
 */
const char* slog_version(int min)
{
    static char verstr[128];

    if (min) 
    {   /* Get only version numbers. (eg. 1.4.85) */
        sprintf(verstr, "%d.%d.%d", SLOGVERSION_MAJOR, SLOGVERSION_MINOR, SLOGBUILD_NUM);
    }
    else 
    {   /* Get version in full format. eg 1.4 build 85 (Jan 21 2017). */
        sprintf(verstr, "%d.%d build %d (%s)", 
            SLOGVERSION_MAJOR, SLOGVERSION_MINOR, SLOGBUILD_NUM, __DATE__);
    }

    return verstr;
}

/*
 * Colorize the given string (str).
 */
char* strclr(const char* clr, char* str, ...)
{
    static char output[MAXMSG];
    char string[MAXMSG];

    /* Read args. */
    va_list args;
    va_start(args, str);
    vsprintf(string, str, args);
    va_end(args);

    /* Colorize string. */
    sprintf(output, "%s%s%s", clr, string, CLR_RESET);

    return output;
}

/*
 * Append log info to log file.
 */
void slog_to_file(char *out, const char *fname, SlogDate *sdate)
{
    char filename[PATH_MAX];

    if (slg.filestamp)
    {   /* Create log filename with date. (eg example-2017-01-21.log) */
        snprintf(filename, sizeof(filename), "%s-%02d-%02d-%02d.log", 
            fname, sdate->year, sdate->mon, sdate->day);
    }
    else 
    {   /* Create log filename using regular name. (eg example.log) */
        snprintf(filename, sizeof(filename), "%s.log", fname);
    }

    FILE *fp = fopen(filename, "a");
    if (fp == NULL) 
    {
        return;
    }

    /* Append log line to log file. */
    fprintf(fp, "%s", out);

    fclose(fp);
}

/*
 * Read cfg file and parse configuration options.
 */
int parse_config(const char *cfg_name)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int ret = 0;

    fp = fopen(cfg_name, "r");
    if (fp == NULL) 
    {
        return 0;
    }

    /* Reading *.cfg file line-by-line. */
    while ((read = getline(&line, &len, fp)) != -1)
    {
        /* Find level in file. */
        if (strstr(line, "LOGLEVEL") != NULL)
        {
            /* Get max log level to print in stdout. */
            slg.level = atoi(line + 8);
            ret = 1;
        }
        if (strstr(line, "LOGFILELEVEL") != NULL)
        {
            /* Level required to write in file. */
            slg.file_level = atoi(line + 12);
            ret = 1;
        }
        else if (strstr(line, "LOGTOFILE") != NULL)
        {
            /* 
             * Get max log level to write in file.
             * If 0 will not write to file. 
             */
            slg.to_file = atoi(line + 9);
            ret = 1;
        }
        else if (strstr(line, "PRETTYLOG") != NULL)
        {
            /* If 1 will output with color. */
            slg.pretty = atoi(line + 9);
            ret = 1;
        }
        else if (strstr(line, "FILESTAMP") != NULL)
        {
            /* If 1 will add date to log name. */
            slg.filestamp = atoi(line + 9);
            ret = 1;
        }
    }

    /* Cleanup. */
    if (line) 
    {
        free(line);
    }
    fclose(fp);

    return ret;
}

/*
 * Generating string in form:
 * yyyy.mm.dd-HH:MM:SS.UU - (some message).
 */
char* slog_get(SlogDate *pDate, char *msg, ...)
{
    static char output[MAXMSG];
    char string[MAXMSG];

    /* Read args. */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Generate output string with date. */
    sprintf(output, "%02d.%02d.%02d-%02d:%02d:%02d.%02d - %s",
            pDate->year, pDate->mon, pDate->day, pDate->hour,
            pDate->min, pDate->sec, pDate->usec, string);

    return output;
}

/*
 * Log exiting process. We save log in file
 * if LOGTOFILE flag is enabled from config.
 */
void slog(int level, int flag, const char *msg, ...)
{
    /* Lock thread for safe. */
    if (slg.td_safe)
    {
        int rc;
        if ((rc = pthread_mutex_lock(&slog_mutex)))
        {
            printf("[ERROR] <%s:%d> inside %s(): Can not lock mutex: %s\n",
                   __FILE__, __LINE__, __func__, strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    SlogDate mdate;
    char string[MAXMSG];
    char prints[MAXMSG];
    char color[32], alarm[32];
    char *output;

    slog_get_date(&mdate);
    /* Place zero-valued bytes. */
    bzero(string, sizeof(string));
    bzero(prints, sizeof(prints));
    bzero(color, sizeof(color));
    bzero(alarm, sizeof(alarm));



    /* Read args. */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);


    /* Check logging levels. */
    if (!level || level <= slg.level || level <= slg.file_level)
    {
        /* Handle flags. */
        switch (flag)
        {
            case SLOG_LIVE:
                strncpy(color, CLR_NORMAL, sizeof(color));
                strncpy(alarm, "LIVE", sizeof(alarm));
                break;
            case SLOG_INFO:
                strncpy(color, CLR_GREEN, sizeof(color));
                strncpy(alarm, "INFO", sizeof(alarm));
                break;
            case SLOG_WARN:
                strncpy(color, CLR_YELLOW, sizeof(color));
                strncpy(alarm, "WARN", sizeof(alarm));
                break;
            case SLOG_DEBUG:
                strncpy(color, CLR_BLUE, sizeof(color));
                strncpy(alarm, "DEBUG", sizeof(alarm));
                break;
            case SLOG_ERROR:
                strncpy(color, CLR_RED, sizeof(color));
                strncpy(alarm, "ERROR", sizeof(alarm));
                break;
            case SLOG_FATAL:
                strncpy(color, CLR_RED, sizeof(color));
                strncpy(alarm, "FATAL", sizeof(alarm));
                break;
            case SLOG_PANIC:
                strncpy(color, CLR_RED, sizeof(color));
                strncpy(alarm, "PANIC", sizeof(alarm));
                break;
            case SLOG_NONE:
                strncpy(prints, string, sizeof(string));
                break;
            default:
                strncpy(prints, string, sizeof(string));
                flag = SLOG_NONE;
                break;
        }

        /* Print output. */
        if (level <= slg.level || slg.pretty)
        {
            if (flag != SLOG_NONE) 
            {
                sprintf(prints, "[%s] %s", strclr(color, alarm), string);
            }
            if (level <= slg.level) 
            {
                printf("%s", slog_get(&mdate, "%s\n", prints));
            }
        }

        /* Save log in file. */
        if (slg.to_file && level <= slg.file_level)
        {
            if (slg.pretty) 
            {
                output = slog_get(&mdate, "%s\n", prints);
            }
            else
            {
                if (flag != SLOG_NONE) 
                {
                    sprintf(prints, "[%s] %s", alarm, string);
                }

                output = slog_get(&mdate, "%s\n", prints);
            }

            /* Add log line to file. */
            slog_to_file(output, slg.fname, &mdate);
        }
    }

    /* Unlock mutex. */
    if (slg.td_safe)
    {
        int rc;
        if ((rc = pthread_mutex_unlock(&slog_mutex)))
        {
            printf("[ERROR] <%s:%d> inside %s(): Can not deinitialize mutex: %s\n",
                   __FILE__, __LINE__, __func__, strerror(rc));
            exit(EXIT_FAILURE);
        }
    }
}


void slog_init(const char* fname, const char* conf, int lvl, int flvl, int t_safe)
{
    int status = 0;

    /* Set up default values. */
    slg.level = lvl;
    slg.file_level = flvl;
    slg.to_file = 0;
    slg.pretty = 0;
    slg.filestamp = 1;
    slg.td_safe = t_safe;

    /* Init mutex sync. */
    if (t_safe)
    {
        /* Init mutex attribute. */
        pthread_mutexattr_t m_attr;
        int rc;
        if ((rc = pthread_mutexattr_init(&m_attr)) ||
                (rc = pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE)) ||
                (rc = pthread_mutex_init(&slog_mutex, &m_attr)) ||
                (rc = pthread_mutexattr_destroy(&m_attr)))
        {
            printf("[ERROR] <%s:%d> inside %s(): Can not initialize mutex: %s\n",
                   __FILE__, __LINE__, __func__, strerror(rc));
            slg.td_safe = 0;
        }
    }

    /* Parse config file. */
    if (conf != NULL)
    {
        slg.fname = fname;
        status = parse_config(conf);
    }

    /* Handle config parser status. */
    if (!status) 
    {
        slog(0, SLOG_INFO, "Initializing logger values without config");
    }
    else 
    {
        slog(0, SLOG_INFO, "Loading logger config from: %s", conf);
    }
}
