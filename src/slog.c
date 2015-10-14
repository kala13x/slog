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

/* Supported colors */
#define CLR_NORM     "\x1B[0m"
#define CLR_RED      "\x1B[31m"
#define CLR_GREEN    "\x1B[32m"
#define CLR_YELLOW   "\x1B[33m"
#define CLR_BLUE     "\x1B[34m"
#define CLR_NAGENTA  "\x1B[35m"
#define CLR_CYAN     "\x1B[36m"
#define CLR_WHITE    "\x1B[37m"
#define CLR_RESET    "\033[0m"

/* Max size of string */
#define MAXMSG 8196

/* Flags */
static SlogFlags slg;
static MutexSync slg_lock;

/* 
 * sync_init - Initialize mutex and set mutex attribute.
 * Argument m_sync is pointer of MutexSync structure.
 */
void sync_init(MutexSync *m_sync) 
{
    /* Set flags */
    m_sync->status_ok = 0;
    m_sync->m_locked = 0;

    /* Init mutex attribute */
    if (pthread_mutexattr_init(&m_sync->m_attr) ||
        pthread_mutexattr_settype(&m_sync->m_attr, PTHREAD_MUTEX_RECURSIVE) ||
        pthread_mutex_init(&m_sync->mutex, &m_sync->m_attr) ||
        pthread_mutexattr_destroy(&m_sync->m_attr))
    {
        slog(0, SLOG_ERROR, "Can not initialize mutex: %d", errno);
        return;
    }

    m_sync->status_ok = 1;
}


/* 
 * sync_destroy - Deitialize mutex and exit if error.
 * Argument m_sync is pointer of MutexSync structure.
 */
void sync_destroy(MutexSync *m_sync)
{
    if (pthread_mutex_destroy(&m_sync->mutex))
    {
        slog(0, SLOG_ERROR, "Can not deinitialize mutex: %d", errno);
        m_sync->status_ok = 0;
        return;
    }
}


/* 
 * sync_lock - Lock mutex and and exit if error.
 * Argument m_sync is pointer of MutexSync structure.
 */
void sync_lock(MutexSync *m_sync)
{
    if (m_sync->status_ok && !m_sync->m_locked) 
    {
        if (pthread_mutex_lock(&m_sync->mutex))
        {
            slog(0, SLOG_ERROR, "Can not lock mutex: %d", errno);
            m_sync->status_ok = 0;
            return;
        }
        m_sync->m_locked = 1;
    }
    else
    {
        slog(0, SLOG_WARN, "Locking bad mutex");
    }
}


/* 
 * sync_lock - Unlock mutex and and exit if error.
 * Argument m_sync is pointer of MutexSync structure.
 */
void sync_unlock(MutexSync *m_sync)
{
    if (m_sync->status_ok && m_sync->m_locked) 
    {
        if (pthread_mutex_unlock(&m_sync->mutex))
        {
            slog(0, SLOG_ERROR, "Can not unlock mutex: %d", errno);
            m_sync->status_ok = 0;
            return;
        }
        m_sync->m_locked = 0;
    }
    else 
    {
        slog(0, SLOG_WARN, "Unlocking bad mutex");
    }
}


/*
 * get_slog_date - Intialize date with system date.
 * Argument is pointer of SlogDate structure.
 */
void get_slog_date(SlogDate *sdate)
{
    time_t rawtime;
    struct tm *timeinfo;
    rawtime = time(NULL);
    timeinfo = localtime(&rawtime);

    /* Get System Date */
    sdate->year = timeinfo->tm_year+1900;
    sdate->mon = timeinfo->tm_mon+1;
    sdate->day = timeinfo->tm_mday;
    sdate->hour = timeinfo->tm_hour;
    sdate->min = timeinfo->tm_min;
    sdate->sec = timeinfo->tm_sec;
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
 * 
 * Color values are:
 *  0 - Normal
 *  1 - Green
 *  2 - Red
 *  3 - Yellow
 *  4 - Blue
 *  5 - Nagenta
 *  6 - Cyan
 *  7 - White
 */
char* strclr(int clr, char* str, ...) 
{
    /* String buffers */
    static char output[MAXMSG];
    char string[MAXMSG];

    /* Read args */
    va_list args;
    va_start(args, str);
    vsprintf(string, str, args);
    va_end(args);

    /* Handle colors */
    switch(clr) 
    {
        case 0:
            sprintf(output, "%s%s%s", CLR_NORM, string, CLR_RESET);
            break;
        case 1:
            sprintf(output, "%s%s%s", CLR_GREEN, string, CLR_RESET);
            break;
        case 2:
            sprintf(output, "%s%s%s", CLR_RED, string, CLR_RESET);
            break;
        case 3:
            sprintf(output, "%s%s%s", CLR_YELLOW, string, CLR_RESET);
            break;
        case 4:
            sprintf(output, "%s%s%s", CLR_BLUE, string, CLR_RESET);
            break;
        case 5:
            sprintf(output, "%s%s%s", CLR_NAGENTA, string, CLR_RESET);
            break;
        case 6:
            sprintf(output, "%s%s%s", CLR_CYAN, string, CLR_RESET);
            break;
        case 7:
            sprintf(output, "%s%s%s", CLR_WHITE, string, CLR_RESET);
            break;
        default:
            return NULL;
    }

    /* Return output */
    return output;
}


/*
 * log_to_file - Save log in file. Argument aut is string which
 * we want to log. Argument fname is log file path and sdate is
 * SlogDate structure variable, we need it to create filename.
 */
void log_to_file(char *out, const char *fname, SlogDate *sdate)
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
char* ret_slog(char *msg, ...) 
{
    /* Used variables */
    static char output[MAXMSG];
    char string[MAXMSG];
    SlogDate mdate;

    /* initialise system date */
    get_slog_date(&mdate);

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Generate output string with date */
    sprintf(output, "%02d.%02d.%02d-%02d:%02d:%02d - %s", 
        mdate.year, mdate.mon, mdate.day, mdate.hour, 
        mdate.min, mdate.sec, string);

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
    /* Used variables */
    SlogDate mdate;
    char string[MAXMSG];
    char prints[MAXMSG];
    char wfiles[MAXMSG];
    char *output;

    /* Lock for safe */
    sync_lock(&slg_lock);

    /* Initialise system date */
    get_slog_date(&mdate);

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Check logging levels */
    if(!level || level <= slg.level)
    {
        /* Handle flags */
        switch(flag) {
            case 1:
                sprintf(prints, "[LIVE]  %s", string);
                strcpy(wfiles, prints);
                break;
            case 2:
                sprintf(prints, "[%s]  %s", strclr(1, "INFO"), string);
                sprintf(wfiles, "[INFO]  %s", string);
                break;
            case 3:
                sprintf(prints, "[%s]  %s", strclr(3, "WARN"), string);
                sprintf(wfiles, "[WARN]  %s", string);
                break;
            case 4:
                sprintf(prints, "[%s] %s", strclr(4, "DEBUG"), string);
                sprintf(wfiles, "[DEBUG] %s", string);
                break;
            case 5:
                sprintf(prints, "[%s] %s", strclr(2, "ERROR"), string);
                sprintf(wfiles, "[ERROR] %s", string);
                break;
            case 6:
                sprintf(prints, "[%s] %s", strclr(2, "FATAL"), string);
                sprintf(wfiles, "[FATAL] %s", string);
                break;
            case 7:
                sprintf(prints, "[%s] %s", strclr(6, "PANIC"), string);
                sprintf(wfiles, "[PANIC] %s", string);
                break;
            case 8:
                sprintf(prints, "%s", string); strcpy(wfiles, prints);
                break;
            default:
                break;
        }

        /* Print output */
        printf("%s", ret_slog("%s\n", prints));

        /* Save log in file */
        if (slg.to_file)
        {
            if (slg.pretty) output = ret_slog("%s\n", prints);
            else output = ret_slog("%s\n", wfiles);

            /* Add log line to file */
            log_to_file(output, slg.fname, &mdate);
        }
    }

    /* Done, unlock mutex */
    sync_unlock(&slg_lock);
}


/*
 * Initialize slog library. Function parses config file and reads log 
 * level and save to file flag from config. First argument is file name 
 * where log will be saved and second argument conf is config file path 
 * to be parsedand third argument lvl is log level for this message.
 */
void init_slog(const char* fname, const char* conf, int lvl)
{
    int status = 0;

    /* Set up default values */
    slg.level = lvl;
    slg.to_file = 0;
    slg.pretty = 0;
    slg.filestamp = 1;

    /* Parse config file */
    if (conf != NULL) 
    {
        slg.fname = fname;
        status = parse_config(conf);
    }

    /* Initialize locker */
    sync_init(&slg_lock);

    /* Handle config parser status */
    if (!status) slog(0, SLOG_INFO, "Initializing logger values without config");
    else slog(0, SLOG_INFO, "Loading logger config from: %s", conf);
}
