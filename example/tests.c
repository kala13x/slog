/*
 *  example/tests.c
 *
 *  2015 - 2020 (c) Sun Dro (f4tb0y@protonmail.com)
 *
 *  This source file is a part of the "slog" project
 *  Read LICENSE file for more details about copyright
 *
 * Implementation tests for slog library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <slog.h>

int test_slog_basic()
{
    printf("Running test_slog_basic...\n");
    slog_init("test_log", SLOG_FLAGS_ALL, 0);
    slog_info("This is an info log");
    slog_warn("This is a warning log");
    slog_error("This is an error log");
    slog_destroy();
    printf("test_slog_basic passed.\n\n");
    return 0;
}

int test_slog_file_logging()
{
    printf("Running test_slog_file_logging...\n");

    slog_config_t config;
    slog_init("file_test_log", SLOG_FLAGS_ALL, 0);
    slog_config_get(&config);
    config.nToFile = 1;
    strncpy(config.sFilePath, "./", sizeof(config.sFilePath));
    slog_config_set(&config);

    char file_path[PATH_MAX];
    slog_get_full_path(file_path, sizeof(file_path));

    slog_info("Logging to file test_log");
    slog_destroy();
    
    FILE *log_file = fopen(file_path, "r");
    if (log_file)
    {
        fclose(log_file);
        printf("File logging test passed.\n\n");
        return 0;
    }
    else
    {
        printf("File logging test FAILED.\n\n");
        return 1;
    }
}

int test_slog_formatting()
{
    printf("Running test_slog_formatting...\n");
    slog_init("format_test_log", SLOG_FLAGS_ALL, 0);
    
    slog_info("Formatted message: %d, %s, %.2f", 42, "hello", 3.1415);
    
    slog_destroy();
    printf("test_slog_formatting passed.\n\n");
    return 0;
}

int main()
{
    int failed = 0;
    
    failed += test_slog_basic();
    failed += test_slog_file_logging();
    failed += test_slog_formatting();

    if (failed > 0)
    {
        printf("One or more tests failed.\n");
        return 1;  // Fail the GitHub Actions pipeline
    }

    printf("All tests passed successfully.\n");
    return 0;  // Pass the GitHub Actions pipeline
}
