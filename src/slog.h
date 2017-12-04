/*
 * The MIT License (MIT)
 *
 *  Copyleft (C) 2015  Sun Dro (a.k.a. kala13x)
 *  Copyleft (C) 2017  George G. Gkasdrogkas (a.k.a. GeorgeGkas)
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


#ifndef __SLOG_H__
#define __SLOG_H__


/* If include header in CPP code. */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <pthread.h>


/* 
 * SOURCE_THROW_LOCATION macro returns string which 
 * points to the file, as well as, the corresponding line 
 * of the caller function.
 */
#define LVL1(x) #x
#define LVL2(x) LVL1(x)
#define SOURCE_THROW_LOCATION "<"__FILE__":"LVL2(__LINE__)"> -- "


/* 
 * Define macros to allow us get further informations 
 * on the corresponding erros. These macros used as wrappers 
 * for slog() function.
 */
#define slog_none(LEVEL, ...) \
    slog(LEVEL, SLOG_NONE, __VA_ARGS__);

#define slog_live(LEVEL, ...) \
    slog(LEVEL, SLOG_LIVE, __VA_ARGS__);

#define slog_info(LEVEL, ...) \
    slog(LEVEL, SLOG_INFO, __VA_ARGS__);

#define slog_warn(LEVEL, ...) \
    slog(LEVEL, SLOG_WARN, SOURCE_THROW_LOCATION __VA_ARGS__);

#define slog_debug(LEVEL, ...) \
    slog(LEVEL, SLOG_DEBUG, __VA_ARGS__);

#define slog_error(LEVEL, ...) \
    slog(LEVEL, SLOG_ERROR, SOURCE_THROW_LOCATION __VA_ARGS__);

#define slog_fatal(LEVEL, ...) \
    slog(LEVEL, SLOG_FATAL, SOURCE_THROW_LOCATION __VA_ARGS__);

#define slog_panic(LEVEL, ...) \
    slog(LEVEL, SLOG_PANIC, SOURCE_THROW_LOCATION __VA_ARGS__);


/* Definitions for version informations. */
#define SLOGVERSION_MAJOR   1
#define SLOGVERSION_MINOR   5
#define SLOGBUILD_NUM       1


/* If compiled on DARWIN/Apple platforms. */
#ifdef DARWIN
#define CLOCK_REALTIME      0x2d4e1588
#define CLOCK_MONOTONIC     0x0
#endif /* DARWIN */


/* Loging flags. */
#define SLOG_NONE   0
#define SLOG_LIVE   1
#define SLOG_INFO   2
#define SLOG_WARN   3
#define SLOG_DEBUG  4
#define SLOG_ERROR  5
#define SLOG_FATAL  6
#define SLOG_PANIC  7


/* Supported colors. */
#define CLR_NORMAL  "\x1B[0m"
#define CLR_RED     "\x1B[31m"
#define CLR_GREEN   "\x1B[32m"
#define CLR_YELLOW  "\x1B[33m"
#define CLR_BLUE    "\x1B[34m"
#define CLR_NAGENTA "\x1B[35m"
#define CLR_CYAN    "\x1B[36m"
#define CLR_WHITE   "\x1B[37m"
#define CLR_RESET   "\033[0m"


/* Flags */
typedef struct {
    const char* fname;
    short file_level;
    short level;
    short to_file;
    short pretty;
    short filestamp;
    short td_safe;
} SlogFlags;


/* Date-time variables. */
typedef struct {
    int year;
    int mon;
    int day;
    int hour;
    int min;
    int sec;
    int usec;
} SlogDate;


/*
 * FUNCTION: slog_version.
 * DESCRIPTION: Get slog library version.
 * PARAM: (min) is flag for output format.
 *        If (min) is 1, function returns version in full format. (eg 1.4 build 85 (Jan 21 2017))
 *        If (min) is 0 function returns only version numbers. (eg. 1.4.85)
 * RETURN: Version and build number of slog library.
 */
const char* slog_version(int min);


/*
 * FUNCTION: strclr.
 * DESCRIPTION: Colorize the given string.
 * PARAMS: (clr) is color value defined above. If it is invalid, function returns NULL.
 *         (str) is string with va_list of arguments which one we want to colorize.
 * RETURN: The colorized string.
 */
char* strclr(const char* clr, char* str, ...);


/*
 * FUNCTION: slog_get.
 * DESCRIPTION: Create a slog formated string.
 * PARAMS: (pDate) holds the current date-time format.
 *         (msg) is string that holds informations about the log action.
 * RETURN: Generating string in form:
 *         yyyy.mm.dd-HH:MM:SS.UU - (some message)
 */
char* slog_get(SlogDate *pDate, char *msg, ...);


/*
 * FUNCTION: slog.
 * DESCRIPTION: Log exiting process. We save log in file if LOGTOFILE flag
 *              is enabled from config.
 * PARAMS: (level) logging level.
 *         (flag) is slog flag defined above.
 *         (msg) is the user defined message for the current log action.
 * RETURN: (void)
 */
void slog(int level, int flag, const char *msg, ...);


/*
 * FUNCTION: slog_init.
 * DESCRIPTION: Function parses config file, reads log level and save
 * to file flag from config.
 * PARAMS: (fname) log file name where log informations will be saved.
 *         (conf) config file path to be parsed. If is NULL the default values are set.
 *         (lvl) log level. If you will not initialize slog, it will only
 *         print messages with log level 0.
 *         (flvl) same as above with the difference that corresponds to file level.
 *         (t_safe) thread safety flag (1 enabled, 0 disabled).
 * RETURN: (void)
 */
void slog_init(const char* fname, const char* conf, int lvl, int flvl, int t_safe);


/* If include header in CPP code. */
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __SLOG_H__ */
