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


/* Flags */
typedef struct {
    char* fname;
    int level;
    int to_file;
} slog_flags;


/* Date variables */
typedef struct {
    int year; 
    int mon; 
    int day;
    int hour;
    int min;
    int sec;
} SystemDate;


/* Definations for version info */
#define SLOGVERSION_MAX 1
#define SLOGVERSION_MIN 0
#define SLOGBUILD 49


/* 
 * Get library version. Function returns version and build number of slog 
 * library. Return value is char pointer. Argument min is flag for output 
 * format. If min is 0, function returns version in full  format, if flag 
 * is 1 function returns only version numbers, For examle: 1.0.52.
-*/
const char* slog_version(int min);


/*
 * Initialize slog library. Function parses config file and reads log 
 * level and save to file flag from config. First argument is file name 
 * where log will be saved and second argument conf is config file path 
 * to be parsedand third argument lvl is log level for this message.
 */
void init_slog(char* fname, char *conf, int lvl);


/*
 * Retunr string in slog format. Function takes arguments 
 * and returns string in slog format without printing and 
 * saveing in file. Return value is char pointer.
 */
char* ret_slog(char *msg, ...);


/*
 * Log exiting process. Function takes arguments and saves 
 * logs in file if LOGTOFILE flag is enabled from config. 
 * Otherwise it just prints log without saveing in file.
 */
void slog(int level, char *msg, ...);


/* For include header in CPP code */
#ifdef __cplusplus
}
#endif
