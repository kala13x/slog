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
#include <stdarg.h>
#include <limits.h>
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
 * format. If min is 0, function returns version in full  format, if flag
 * is 1 function returns only version number, For examle: 1.3.0
 */
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
 *  8 - No Color (returns input string)
 */
char* strclr(int clr, char* str, ...)
{
    /* String buffers */
    static char output[MAXMSG];
    char string[MAXMSG];
    char *colorstr, *resetstr;

    /* Read args */
    va_list args;
    va_start(args, str);
    vsprintf(string, str, args);
    va_end(args);

    /* Handle colors */
    resetstr = CLR_RESET;
    switch(clr)
    {
        case 0:
            colorstr = CLR_NORM; break;
        case 1:
            colorstr = CLR_GREEN; break;
        case 2:
            colorstr = CLR_RED; break;
        case 3:
            colorstr = CLR_YELLOW; break;
        case 4:
            colorstr = CLR_BLUE; break;
        case 5:
            colorstr = CLR_NAGENTA; break;
        case 6:
            colorstr = CLR_CYAN; break;
        case 7:
            colorstr = CLR_WHITE; break;
        case 8:
            colorstr = ""; resetstr = ""; break;
        default:
            return NULL;
    }
    
    /* Return output */
    snprintf(output, sizeof(output), "%s%s%s", colorstr, string, resetstr);
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
    else
        snprintf(filename, sizeof(filename), "%s", fname);

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
            /* Get log level */
            slg.level = atoi(line+8);
            ret = 1;
        }
        else if(strstr(line, "LOGTOFILE") != NULL)
        {
            /* Get log file enable/disable */
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
            /* Get log type */
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
    SlogDate sdate;

    /* initialise system date */
    get_slog_date(&sdate);

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Generate output string with date */
    sprintf(output, "%02d.%02d.%02d-%02d:%02d:%02d - %s",
        sdate.year, sdate.mon, sdate.day, sdate.hour,
        sdate.min, sdate.sec, string);

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
    SlogDate sdate;
    char string[MAXMSG];
    char prints[MAXMSG];
    char *output;
    char *tag;
    int  color=0;

    /* Initialise system date */
    get_slog_date(&sdate);

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Check logging levels */
    if(level <= slg.level)
    {
        /* Handle flags */
        switch(flag) {
            case SLOG_LIVE:
                tag = "LIVE "; color = 8;
                break;
            case SLOG_INFO:
                tag = "INFO "; color = 1;
                break;
            case SLOG_WARN:
                tag = "WARN "; color = 3;
                break;
            case SLOG_DEBUG:
                tag = "DEBUG"; color = 4;
                break;
            case SLOG_ERROR:
                tag = "ERROR"; color = 2;
                break;
            case SLOG_FATAL:
                tag = "FATAL"; color = 2;
                break;
            case SLOG_NONE:
            default:
                tag = ""; color = 8;
        }

        /* Print output */
        snprintf(prints, sizeof(prints), "[%s] %s", strclr(color, tag), string);
        printf("%s", ret_slog("%s\n", prints));

        /* Save log in file */
        if (slg.to_file)
        {
            if (!slg.pretty)
                snprintf(prints, sizeof(prints), "[%s] %s", tag, string);
            output = ret_slog("%s\n", prints);

            /* Add log line to file */
            log_to_file(output, slg.fname, &sdate);
        }
    }
}


/*
 * Initialize slog library. Function parses config file and reads log
 * level and save to file flag from config. First argument is file name
 * where log will be saved and second argument conf is config file path
 * to be parsed and third argument lvl is log level for this message.
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

    /* Handle config parser status */
    if (!status) slog(0, SLOG_INFO, "Initializing logger values without config");
    else slog(0, SLOG_INFO, "Loading logger config from: %s", conf);
}
