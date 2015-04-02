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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define VERSION "0.2.1 Snapshot"
#define BUILD 14


/*---------------------------------------------
| Structure for log level
---------------------------------------------*/
typedef struct {
	char* fname;
	int level;
	int l_max;
	int to_file;
} SLogValues;


/*---------------------------------------------
| Structure of date variables
---------------------------------------------*/
typedef struct {
    int year; 
    int mon; 
    int day;
    int sec;
} SystemDate;


/*---------------------------------------------
| Get slog version
---------------------------------------------*/
const char* slog_version();


/*---------------------------------------------
| Initialise log level
---------------------------------------------*/
void init_slog(char* fname, int to_file, int max);


/*---------------------------------------------
| Log exiting process
---------------------------------------------*/
void slog(int level, char *msg, ...);