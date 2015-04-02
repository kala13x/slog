/*-----------------------------------------------------
Name: log.c
Date: 2015.04.01
Auth: kala13x (a.k.a 7th Ghost)
Desc: Log and print exiting status
-----------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "slog.h"
#include "parser.h"

#define MAXMSG 8196


/*---------------------------------------------
| Initialise log level
---------------------------------------------*/
void init_slog(LogValues *val, char* fname, int max) 
{
    val->level = 0;
    val->fname = strdup(fname);
    val->l_max = max;
}


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
static int handler(void *config, const char *section, const char *name,
                   const char *value)
{
    LogValues *val = (LogValues*)config;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("config", "log")) {
        val->level = atoi(value);
    } else {
        return 0;
    }

    return 1;
}


/*---------------------------------------------
| Log exiting process
---------------------------------------------*/
void slog(int level, char *msg, ...) 
{
    /* Used variables */
    char output[MAXMSG];
    char string[MAXMSG];
    LogValues val;
    SystemDate mdate;

    /* Initialise log level */
    init_slog(&val, "chvenvr", 3);

    /* initialise system date */
    init_date(&mdate);

    /* Parse config file */
    if (ini_parse("config.cfg", handler, &val) < 0) 
    {
        printf("%02d:%02d:%02d:%02d - [ERROR] - Can not parse file: 'config.cfg'\n", 
            mdate.year, mdate.mon, mdate.day, mdate.sec);
        return;
    }

    /* Read args */
    va_list args;
    va_start(args, msg);
    vsprintf(string, msg, args);
    va_end(args);

    /* Check logging levels */
    if(!level || level == val.level && level <= val.l_max) 
    {
        /* Generate output string with date */
        sprintf(output, "%02d:%02d:%02d:%02d - %s\n", 
                mdate.year, mdate.mon, mdate.day, mdate.sec, string);

        /* Print output */
        printf("%s", output);

        /* Save log in file */
        log_to_file(output, val.fname, &mdate);
    }
}