/*
 * The MIT License (MIT)
 *  
 *  Copyleft (C) 2015  Sun Dro (a.k.a. kala13x)
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


/* Max size of string */
#define MAXMSG 8196

/* Flags */
static SlogFlags slg;
static pthread_mutex_t slog_mutex;

#ifdef DARWIN

static inline int clock_gettime(int clock_id, struct timespec *ts)
{
    struct timeval tv;

    if (clock_id != CLOCK_REALTIME) {
        errno = EINVAL;
        return (-1);
    }
    if (gettimeofday(&tv, NULL) < 0)
        return (-1);
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
    return (0);
}
#endif // DARWIN

/*
 * slog_get_date - Intialize date with system date.
 * Argument is pointer of SlogDate structure.
 */
void slog_get_date(SlogDate *sdate)
{
    time_t rawtime;
    struct tm timeinfo;
    struct timespec now;
    rawtime = time(NULL);
    localtime_r(&rawtime, &timeinfo);

    /* Get System Date */
    sdate->year = timeinfo.tm_year+1900;
    sdate->mon = timeinfo.tm_mon+1;
    sdate->day = timeinfo.tm_mday;
    sdate->hour = timeinfo.tm_hour;
    sdate->min = timeinfo.tm_min;
    sdate->sec = timeinfo.tm_sec;

    /* Get micro seconds */
    clock_gettime(CLOCK_MONOTONIC, &now);
    sdate->usec = now.tv_nsec / 10000000;
}


/* 
 * Get library version. Function returns version and build number of slog 
 * library. Return value is char pointer. Argument min is flag for output 
 * format. If min is 1, function returns version in full  format, if flag 
 * is 0 function returns only version numbers, For examle: 1.3.0
-*/
const char* slog_version(int min)
{
    static char verstr[128];

    /* Version short */
    if (min) sprintf(verstr, "%d.%d.%d", 
        SLOGVERSION_MAX, SLOGVERSION_MIN, SLOGBUILD_NUM);

    /* Version long */
    else sprintf(verstr, "%d.%d build %d (%s)", 
        SLOGVERSION_MAX, SLOGVERSION_MIN, SLOGBUILD_NUM, __DATE__);

    return verstr;
}


/*
 * strclr - Colorize string. Function takes color value and string 
 * and returns colorized string as char pointer. First argument clr 
 * is color value (if it is invalid, function retunrs NULL) and second 
 * is string with va_list of arguments which one we want to colorize.
 */
char* strclr(const char* clr, char* str, ...) 
{
    /* String buffers */
    static char output[MAXMSG];
    char string[MAXMSG];

    /* Read args */
    va_list args;
    va_start(args, str);
    vsprintf(string, str, args);
    va_end(args);

    /* Colorize string */
    sprintf(output, "%s%s%s", clr, string, CLR_RESET);

    return output;
}


/*
 * log_to_file - Save log in file. Argument aut is string which
 * we want to log. Argument fname is log file path and sdate is
 * SlogDate structure variable, we need it to create filename.
 */
void slog_to_file(char *out, const char *fname, SlogDate *sdate)
{
    /* Used variables */
    char filename[PATH_MAX];

    /* Create log filename with date */
    if (slg.filestamp)
    {
        snprintf(filename, sizeof(filename), "%s-%02d-%02d-%02d.log",
            fname, sdate->year, sdate->mon, sdate->day);
    }
    else snprintf(filename, sizeof(filename), "%s.log", fname);

    /* Open file pointer */
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) return;

    /* Write key in file */
    fprintf(fp, "%s", out);

    /* Close file pointer */
    fclose(fp);
}


/*
 * parse_config - Parse config file. Argument cfg_name is path 
 * of config file name to be parsed. Function opens config file 
 * and parses LOGLEVEL and LOGTOFILE flags from it.
 */
int parse_config(const char *cfg_name)
{
    /* Used variables */
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int ret = 0;

    /* Open file pointer */
    file = fopen(cfg_name, "r");
    if(file == NULL) return 0;

    /* Line-by-line read cfg file */
    while ((read = getline(&line, &len, file)) != -1)
    {
        /* Find level in file */
        if(strstr(line, "LOGLEVEL") != NULL)
        {
            /* Get logtofile flag */
            slg.level = atoi(line+8);
            ret = 1;
        }
        if(strstr(line, "LOGFILELEVEL") != NULL)
        {
            /* Get logtofile flag */
            slg.file_level = atoi(line+12);
            ret = 1;
        }
        else if(strstr(line, "LOGTOFILE") != NULL)
        {
            /* Get log level */
            slg.to_file = atoi(line+9);
            ret = 1;
        }
        else if(strstr(line, "PRETTYLOG") != NULL)
        {
            /* Get log type */
            slg.pretty = atoi(line+9);
            ret = 1;
        }
        else if(strstr(line, "FILESTAMP") != NULL)
        {
            /* Get filestamp */
            slg.filestamp = atoi(line+9);
            ret = 1;
        }
    }

    /* Cleanup */
    if (line) free(line);
    fclose(file);

    return ret;
}


/*
 * Retunr string in slog format. Function takes arguments 
 * and returns string in slog format without printing and 
 * saveing in file. Return value is char pointer.
 */
char* slog_get(SlogDate *pDate, char *msg, ...) 
{
    /* Used variables */
    static char output[MAXMSG];
    char string[MAXMSG];

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Generate output string with date */
    sprintf(output, "%02d.%02d.%02d-%02d:%02d:%02d.%02d - %s", 
        pDate->year, pDate->mon, pDate->day, pDate->hour, 
        pDate->min, pDate->sec, pDate->usec, string);

    /* Return output */
    return output;
}


/*
 * slog - Log exiting process. Function takes arguments and saves
 * log in file if LOGTOFILE flag is enabled from config. Otherwise
 * it just prints log without saveing in file. Argument level is
 * logging level and flag is slog flags defined in slog.h header.
 */
void slog(int level, int flag, const char *msg, ...)
{
    /* Lock for safe */
    if (slg.td_safe) 
    {
        if (pthread_mutex_lock(&slog_mutex))
        {
            printf("<%s:%d> %s: [ERROR] Can not lock mutex: %d\n", 
                __FILE__, __LINE__, __FUNCTION__, errno);
            exit(EXIT_FAILURE);
        }
    }

    /* Used variables */
    SlogDate mdate;
    char string[MAXMSG];
    char prints[MAXMSG];
    char color[32], alarm[32];
    char *output;

    slog_get_date(&mdate);
    bzero(string, sizeof(string));
    bzero(prints, sizeof(prints));
    bzero(color, sizeof(color));
    bzero(alarm, sizeof(alarm));

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Check logging levels */
    if(!level || level <= slg.level || level <= slg.file_level)
    {
        /* Handle flags */
        switch(flag) 
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
                strncpy(color, CLR_WHITE, sizeof(color));
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

        /* Print output */
        if (level <= slg.level || slg.pretty)
        {
            if (flag != SLOG_NONE) sprintf(prints, "[%s] %s", strclr(color, alarm), string);
            if (level <= slg.level) printf("%s", slog_get(&mdate, "%s\n", prints));
        }

        /* Save log in file */
        if (slg.to_file && level <= slg.file_level)
        {
            if (slg.pretty) output = slog_get(&mdate, "%s\n", prints);
            else 
            {
                if (flag != SLOG_NONE) sprintf(prints, "[%s] %s", alarm, string);
                output = slog_get(&mdate, "%s\n", prints);
            } 

            /* Add log line to file */
            slog_to_file(output, slg.fname, &mdate);
        }
    }

    /* Done, unlock mutex */
    if (slg.td_safe) 
    {
        if (pthread_mutex_unlock(&slog_mutex)) 
        {
            printf("<%s:%d> %s: [ERROR] Can not deinitialize mutex: %d\n", 
                __FILE__, __LINE__, __FUNCTION__, errno);
            exit(EXIT_FAILURE);
        }
    }
}


/*
 * Initialize slog library. Function parses config file and reads log 
 * level and save to file flag from config. First argument is file name 
 * where log will be saved and second argument conf is config file path 
 * to be parsedand third argument lvl is log level for this message.
 */
void slog_init(const char* fname, const char* conf, int lvl, int flvl, int t_safe)
{
    int status = 0;

    /* Set up default values */
    slg.level = lvl;
    slg.file_level = flvl;
    slg.to_file = 0;
    slg.pretty = 0;
    slg.filestamp = 1;
    slg.td_safe = t_safe;

    /* Init mutex sync */
    if (t_safe)
    {
        /* Init mutex attribute */
        pthread_mutexattr_t m_attr;
        if (pthread_mutexattr_init(&m_attr) ||
            pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE) ||
            pthread_mutex_init(&slog_mutex, &m_attr) ||
            pthread_mutexattr_destroy(&m_attr))
        {
            printf("<%s:%d> %s: [ERROR] Can not initialize mutex: %d\n", 
                __FILE__, __LINE__, __FUNCTION__, errno);
            slg.td_safe = 0;
        }
    }

    /* Parse config file */
    if (conf != NULL) 
    {
        slg.fname = fname;
        status = parse_config(conf);
    }

    /* Handle config parser status */
    if (!status) slog(0, SLOG_INFO, "Initializing logger values without config");
    else slog(0, SLOG_INFO, "Loading logger config from: %s", conf);
}
