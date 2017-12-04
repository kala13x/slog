/*
 *  example.c
 *  Copyleft (C) 2015  Sun Dro (a.k.a 7th Ghost)
 *  Copyleft (C) 2017  George G. Gkasdrogkas (a.k.a. GeorgeGkas)
 *
 *  Source example of slog library usage. Use GCC.
 */

#include <stdio.h>
#include <string.h>
#include <slog.h>

void seperator() {
    for (int i = 1; i <= 68; ++i) 
    {
        printf("=");
    }
    printf("\n");
}

void greet()
{
    /* Get and print slog version. */
    seperator();
    printf("slog Version: %s\n", slog_version(0));
    seperator();
}

int main()
{
    /* Used variables. */
    char char_arg[32];
    int int_arg;

    /* Init args */
    strcpy(char_arg, "test string");
    int_arg = 69;

    /* Greet users. */
    greet();

    /*
    * slog_init - Initialize slog.
    * First argument is log filename.
    * Second argument is config file.
    * Third argument is max log level on console.
    * Fourth is max log level on file.
    * Fifth is thread safe flag.
    */
    slog_init("example", "slog.cfg", 2, 3, 1);

    /* Log and print something with level 0. */
    slog_live(0, "Test message with level 0");

    /* Log and print something with level 1. */
    slog_warn(1, "Warn message with level 1");

    /* Log and print something with level 2. */
    slog_info(2, "Info message with level 2");

    /* Log and print something with level 3. */
    slog_live(3, "Test message with level 3");

    /* Log and print something with char argument */
    slog_debug(0, "Debug message with char argument: %s", char_arg);

    /* Log and print something with int argument. */
    slog_error(0, "Error message with int argument: %d", int_arg);

    /* Log and print something fatal. */
    slog_fatal(0, "Fatal message. We fell into dead zone.");

    /* Log and print something in panic mode. */
    slog_panic(0, "Panic here! We don't have tim....");

    /*
     * Test log with higher level than log max value
     * This will never be printed while log level argument is higher than max log level.
     */
    slog_none(4, "[LIVE] Test log with higher level than log max value");

    /* Print something with our own colorized line. */
    slog_none(0, "[%s] This is our own colorized string", strclr(CLR_NAGENTA, "TEST"));

    return 0;
}
