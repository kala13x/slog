/*
 *  example.c 
 *  Copyleft (C) 2015  Sun Dro (a.k.a 7th Ghost)
 *
 * Source example of slog library usage. Use GCC.
 */

#include <stdio.h>
#include <string.h>
#include "src/slog.h"

int main()
{
    /* Used variables */
    char char_arg[32];
    int int_arg;

    /* Init args */
    strcpy(char_arg, "test string");
    int_arg = 69;

    /* 
     * init_slog - Initialise slog 
     * First argument is log filename 
     * Second argument is config file
     * Third argument is max log level 
     */
    init_slog("example", "slog.cfg", 3);

    /* Get and print slog version */
    slog(0, SLOG_NONE, "=========================================");
    slog(0, SLOG_NONE, "sLog Version: %s", slog_version(0));
    slog(0, SLOG_NONE, "=========================================");

    /* Log and print something with level 0 */
    slog(0, SLOG_LIVE, "Test message with level 0");

    /* Log and print something with level 1 */
    slog(1, SLOG_WARN, "Warn message with level 1");

    /* Log and print something with level 2 */
    slog(2, SLOG_INFO, "Info message with level 2");

    /* Log and print something with level 3 */
    slog(3, SLOG_LIVE, "Test message with level 3");

    /* Log and print something with char argument */
    slog(0, SLOG_DEBUG, "Debug message with char argument: %s", char_arg);

    /* Log and print something with int argument */
    slog(0, SLOG_ERROR, "Error message with int argument: %d", int_arg);

    /* Test log with higher level than log max value 
    * This will never be printed while log level argument is higher than max log level */
    slog(4, SLOG_NONE, "[LIVE] Test log with higher level than log max value");

    return 0;
}
