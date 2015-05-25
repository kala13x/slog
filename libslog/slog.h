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


/* For include header in CPP code */
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Definations for version info */
#define SLOGVERSION "0.2.3 Snapshot"
#define SLOGBUILD 28


/* Structure for log level */
typedef struct {
    char* fname;
    int level;
    int l_max;
    int to_file;
} SLogValues;


/* Structure of date variables */
typedef struct {
    int year; 
    int mon; 
    int day;
    int hour;
    int min;
    int sec;
} SystemDate;


/* 
 * Get library version. Function returns version and build number of 
 * slog library. Return value is static char pointer.
-*/
const char* slog_version();


/*
 * Initialize slog library. Function parses slog.cfg file
 * and reads loggin level and save to file flag from it.
 *
 * Arguments are:
 * @ First argument is file name where log will be saved
 * @ Second argument is maximum allowed log level
 */
void init_slog(char* fname, int max);


/*
 * Retunr string in slog format. Function takes arguments and
 * returns string in slog format without saveing in file.
 */
char* ret_slog(char *msg, ...);


/*
 * Log exiting process. Function takes arguments and
 * logs process in log file if log to file flag is enabled.
 * Otherwise it prints log with stdout.
 */
void slog(int level, char *msg, ...);


/* For include header in CPP code */
#ifdef __cplusplus
}
#endif
