/*---------------------------------------------------------------------------

 slog is Advanced logging library for C/C++

 Copyright (c) 2015 Sun Dro (a.k.a. 7th Ghost)
 Web: http://off-sec.com/ ; E-Mail: kala0x13@gmail.com

 This is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This software is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

---------------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "slog.h"

/* Max buffer size of message */
#define MAXMSG 8196

/* Main struct variable */
SLogValues slog_val;


/*---------------------------------------------
| Initialise Date
---------------------------------------------*/
void init_date(SystemDate *mdate) 
{
    time_t rawtime;
    struct tm *timeinfo;
    rawtime = time(NULL);
    timeinfo = localtime(&rawtime);

    /* Get System Date */
    mdate->year = timeinfo->tm_year+1900;
    mdate->mon = timeinfo->tm_mon+1;
    mdate->day = timeinfo->tm_mday;
    mdate->sec = timeinfo->tm_sec;
}


/*---------------------------------------------
| Get slog version
---------------------------------------------*/
const char* slog_version()
{
    static char verstr[128];
    sprintf(verstr, "%s Build %d", VERSION, BUILD);
    return verstr;
}


/*---------------------------------------------
| Save log in file
---------------------------------------------*/
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


/*---------------------------------------------
| Parse config file
---------------------------------------------*/
int parse_config(char *cfg_name)
{
    /* Used variables */
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    /* Open file pointer */
    file = fopen(cfg_name, "r");
    if(file == NULL) return 1;

    /* Line-by-line read cfg file */
    while ((read = getline(&line, &len, file)) != -1) 
    {
        /* Find level in file */
        if(strstr(line, "log") != NULL) 
        {
            /* Get log level */
            slog_val.level = atoi(line+3);

            /* Close file and return */
            fclose(file);
            return 0;
        }
    } 

    return 1;
}


/*---------------------------------------------
| Initialise log level
---------------------------------------------*/
void init_slog(char* fname, int to_file, int max) 
{
    slog_val.level = 0;
    slog_val.fname = strdup(fname);
    slog_val.l_max = max;
    slog_val.to_file = to_file;

    /* Parse config file */
    if (parse_config("slog.cfg")) 
    {
        printf("[ERROR] - Cannot parse file: 'slog.cfg'\n");
        return;
    }
}


/*---------------------------------------------
| Log exiting process
---------------------------------------------*/
void slog(int level, char *msg, ...) 
{
    /* Used variables */
    char output[MAXMSG];
    char string[MAXMSG];
    SystemDate mdate;

    /* initialise system date */
    init_date(&mdate);

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Check logging levels */
    if(!level || level <= slog_val.level && level <= slog_val.l_max) 
    {
        /* Generate output string with date */
        sprintf(output, "%02d:%02d:%02d:%02d - %s\n", 
                mdate.year, mdate.mon, mdate.day, mdate.sec, string);

        /* Print output */
        printf("%s", output);

        /* Save log in file */
        if (slog_val.to_file) 
            log_to_file(output, slog_val.fname, &mdate);
    }
}