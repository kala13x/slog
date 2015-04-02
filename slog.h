/*-----------------------------------------------------
Name: log.h
Date: 2015.04.01
Auth: kala13x (a.k.a 7th Ghost)
Desc: Header file of logging system
-----------------------------------------------------*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


/*---------------------------------------------
| Structure for log level
---------------------------------------------*/
typedef struct {
	char* fname;
	int level;
	int l_max;
} LogValues;


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
| Initialise log level
---------------------------------------------*/
void init_slog(LogValues *val, char* fname, int max);


/*---------------------------------------------
| Log exiting process
---------------------------------------------*/
void slog(int level, char *msg, ...);