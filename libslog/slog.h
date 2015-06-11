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


/* Definations for version info */
#define SLOGVERSION_MAX  1
#define SLOGVERSION_MIN  2
#define SLOGBUILD_NUM    61


/* Loging flags */
#define SLOG_LIVE   1
#define SLOG_INFO   2
#define SLOG_WARN   3
#define SLOG_DEBUG  4
#define SLOG_ERROR  5
#define SLOG_NONE   6


/* Date variables */
typedef struct {
    int year;
    int mon;
    int day;
    int hour;
    int min;
    int sec;
} SystemDate;


/* Flags */
typedef struct {
    const char* fname;
    int level;
    int to_file;
} slog_flags;


/*
 * Get library version. Function returns version and build number of slog
 * library. Return value is char pointer. Argument min is flag for output
 * format. If min is 0, function returns version in full  format, if flag
 * is 1 function returns only version number, For examle: 1.3.0
 */
const char* slog_version(int min);


/*
 * strclr - Colorize string. Function takes color value and string
 * and returns colorized string as char pointer. First argument clr
 * is color value (if it is invalid, function retunrs NULL) and second
 * is string with va_list of arguments which one we want to colorize.
 */
char* strclr(int clr, char* str, ...);


/*
 * Initialize slog library. Function parses config file and reads log
 * level and save to file flag from config. First argument is file name
 * where log will be saved and second argument conf is config file path
 * to be parsed and third argument lvl is log level for this message.
 */
void init_slog(const char* fname, const char *conf, int lvl);


/*
 * Return string in slog format. Function takes arguments
 * and returns string in slog format without printing and
 * saveing in file. Return value is char pointer.
 */
char* ret_slog(char *msg, ...);


/*
 * slog - Log exiting process. Function takes arguments and saves
 * log in file if LOGTOFILE flag is enabled from config. Otherwise
 * it just prints log without saveing in file. Argument level is
 * logging level and flag is slog flags defined in slog.h header.
 */
void slog(int level, int flag, const char *msg, ...);


/* For include header in CPP code */
#ifdef __cplusplus
}
#endif
