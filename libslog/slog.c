/*
 *  slog is Advanced logging library for C/C++
 *
 *  Copyright (c) 2015 Sun Dro (a.k.a. 7th Ghost)
 *  Web: http://off-sec.com/ ; E-Mail: kala0x13@gmail.com
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "slog.h"

/* Max buffer size of message */
#define MAXMSG 8196

/* Flags */
static slog_flags slg;


/*
 * get_system_date - Intialize date with system date.
 * Argument is pointer of SystemDate structure.
 */
void get_system_date(SystemDate *mdate) 
{
    time_t rawtime;
    struct tm *timeinfo;
    rawtime = time(NULL);
    timeinfo = localtime(&rawtime);

    /* Get System Date */
    mdate->year = timeinfo->tm_year+1900;
    mdate->mon = timeinfo->tm_mon+1;
    mdate->day = timeinfo->tm_mday;
    mdate->hour = timeinfo->tm_hour;
    mdate->min = timeinfo->tm_min;
    mdate->sec = timeinfo->tm_sec;
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
        SLOGVERSION_MAX, SLOGVERSION_MIN, SLOGBUILD);

    /* Version long */
    else sprintf(verstr, "%d.%d build %d (%s)", 
        SLOGVERSION_MAX, SLOGVERSION_MIN, SLOGBUILD, __DATE__);

    return verstr;
}


/*
 * log_to_file - Save log in file. Argument aut is string which
 * we want to log. Argument fname is log file path and mdate is 
 * SystemDate structure variable, we need it to create filename.
 */
void log_to_file(char *out, char *fname, SystemDate *mdate) 
{
    /* Used variables */
    char filename[32];

    /* Create log filename with date */
    sprintf(filename, "%s-%02d-%02d-%02d.log", 
        fname, mdate->year, mdate->mon, mdate->day);

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
int parse_config(char *cfg_name)
{
    /* Used variables */
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int ret = 1;

    /* Open file pointer */
    file = fopen(cfg_name, "r");
    if(file == NULL) return 1;

    /* Line-by-line read cfg file */
    while ((read = getline(&line, &len, file)) != -1) 
    {
        /* Find level in file */
        if(strstr(line, "LOGLEVEL") != NULL) 
        {
            /* Get log level */
            slg.level = atoi(line+8);
            ret = 0;
        }
        else if(strstr(line, "LOGTOFILE") != NULL) 
        {
            /* Get log level */
            slg.to_file = atoi(line+9);
            ret = 0;
        }
    } 

    /* Close file and return */
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
    SystemDate mdate;

    /* initialise system date */
    get_system_date(&mdate);

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
 * Log exiting process. Function takes arguments and saves 
 * logs in file if LOGTOFILE flag is enabled from config. 
 * Otherwise it just prints log without saveing in file.
 */
void slog(int level, char *msg, ...) 
{
    /* Used variables */
    char string[MAXMSG];
    char *output;
    SystemDate mdate;

    /* initialise system date */
    get_system_date(&mdate);

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Check logging levels */
    if(!level || level <= slg.level)
    {
        /* Get output string with date */
        output = ret_slog("%s\n", string);

        /* Print output */
        printf("%s", output);

        /* Save log in file */
        if (slg.to_file)
            log_to_file(output, slg.fname, &mdate);
    }
}


/*
 * Initialize slog library. Function parses config file and reads log 
 * level and save to file flag from config. First argument is file name 
 * where log will be saved and second argument conf is config file path 
 * to be parsed and third argument lvl is log level for this message.
 */
void init_slog(char* fname, char* conf, int lvl) 
{
    slg.level = lvl;
    slg.fname = fname;
    slg.to_file = 0;

    /* Parse config file */
    if (parse_config(conf)) 
    {
        slog(0, "[WARNING] LOGLEVEL and/or LOGTOFILE flag is not set from config.");
        return;
    }
}
