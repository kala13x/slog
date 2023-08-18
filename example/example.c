/*
 *  example/example.c
 *
 *  2015 - 2020 (c) Sun Dro (f4tb0y@protonmail.com)
 *
 *  This source file is a part of the "slog" project
 *  Read LICENSE file for more details about copyright
 *
 * Implementation example of SLog library
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <slog.h>

int logCallback(const char *pLog, size_t nLength, slog_flag_t eFlag, void *pCtx)
{
    printf("%s", pLog);
    return 0;
}

void greet() 
{
    /* Get and print slog version */
    char sVersion[128];
    slog_version(sVersion, sizeof(sVersion), 0);
 
    printf("=========================================\n");
    printf("SLog Version: %s\n", sVersion);
    printf("=========================================\n");
}

int main()
{
    /* Used variables */
    slog_config_t cfg;
    int nInteger = 69;
    char sBuffer[12];

    strcpy(sBuffer, "test string");
    uint16_t nLogFlags = SLOG_ERROR | SLOG_NOTAG | SLOG_NOTE;

    /* Greet users */
    greet();

    /* Initialize slog and allow only error and not tagged output */
    slog_init("example", SLOG_FLAGS_ALL, 0);
    slog_config_get(&cfg);

    /* Disable time and date in output */
    cfg.eDateControl = SLOG_TIME_DISABLE;
    slog_config_set(&cfg);

    /* Just simple log without anything (color, tag, thread id)*/
    slog("Simple message without anything");
    slogn("Simple note message without timings");

    /* Enable only time in output */
    cfg.eDateControl = SLOG_TIME_ONLY;
    cfg.nIndent = 1;
    slog_config_set(&cfg);

    slogn("Simple note message");
    slog("Simple message with time only");

    slog_new_line(0); // Disable new line ending
    /* Simple log without adding new line character at the end */
    slog("Simple message with our own new line character\n");
    slog_new_line(1); // Enable new line ending again

    /* Enable all logging flags */
    slog_enable(SLOG_FLAGS_ALL);

    /* Warning message */
    slog_warn("Warning message without variable");

    /* Info message with char*/
    slog_info("Info message with string variable: %s", sBuffer);

    /* Note message */
    slog_note("Note message with integer variable: %d", nInteger);

    /* Trace thread id and print in output */
    cfg.nTraceTid = 1;
    slog_config_set(&cfg);

    /* Debug message with string and integer */
    slog_debug("Debug message with enabled thread id tracing");

    /* Error message with errno string (in this case must be 'Success')*/
    slog_error("Error message with errno string: %s", strerror(errno));

    /* Disable trace tag */
    slog_disable(SLOG_TRACE);

    /* This will never be printed while SLOG_TRACE is disabled by slog_disable() function */
    slog_trace("Test log with disabled tag");

    /* Enable file logger and color the whole log output instead of coloring only tags*/
    cfg.eColorFormat = SLOG_COLORING_FULL;
    cfg.nToFile = 1;
    cfg.nKeepOpen = 1;
    slog_config_set(&cfg);

    /* Print message and save log in the file */
    slog_debug("Debug message in the file with full line color enabled");

    /* Enable trace tag */
    slog_enable(SLOG_TRACE);

    /* Enable log callback */
    slog_callback_set(logCallback, NULL);

    /* We can trace function and line number with and without output message */
    slog_trace("Trace message throws source location");

    /* Fatal error message */
    slog_fatal("Fatal message also throws source location");

    /* Enable date + time in output */ 
    cfg.eDateControl = SLOG_DATE_FULL;
    slog_config_set(&cfg);
    slog_debug("Debug message with time and date");

    /* Disable output coloring*/
    cfg.eColorFormat = SLOG_COLORING_DISABLE;
    slog_config_set(&cfg);
    slog_debug("Disabled output coloring");

    /* Just throw source location without output message */
    slog_trace();
    slog_debug("Above we traced source location without output message");

    slog_destroy();
    return 0;
}
