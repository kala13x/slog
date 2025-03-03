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
#include <pthread.h>
#include <slog.h>

#define TEST_PASSED "\033[0;32mPASSED\033[0m"
#define TEST_FAILED "\033[0;31mFAILED\033[0m"

int test_slog_basic()
{
    printf("=============> Running test_slog_basic...\n");
    slog_init("test_log", SLOG_FLAGS_ALL, 0);
    slog_info("This is an info log");
    slog_warn("This is a warn log");
    slog_error("This is an error log");
    slog_debug("This is a debug log");
    slog_trace("This is a trace log");
    slog_destroy();
    printf("=============> test_slog_basic %s.\n\n", TEST_PASSED);
    return 0;
}

int test_slog_file_logging()
{
    printf("=============> Running test_slog_file_logging...\n");
    slog_init("file_test_log", SLOG_FLAGS_ALL, 0);

    slog_config_t config;
    slog_config_get(&config);
    config.nToFile = 1;
    slog_config_set(&config);

    slog_info("Logging to file test_log");

    char file_path[PATH_MAX];
    slog_get_full_path(file_path, sizeof(file_path));
    slog_destroy();
    
    FILE *log_file = fopen(file_path, "r");
    if (log_file != NULL)
    {
        fclose(log_file);
        printf("=============> File logging test %s.\n\n", TEST_PASSED);
        return 0;
    }
    else
    {
        printf("=============> File logging test %s.\n\n", TEST_FAILED);
        return 1;
    }
}

int test_slog_formatting()
{
    printf("=============> Running test_slog_formatting...\n");
    slog_init("format_test_log", SLOG_FLAGS_ALL, 0);

    slog_info("Formatted message: %d, %s, %.2f", 42, "hello", 3.1415);
    slog_destroy();

    printf("=============> test_slog_formatting %s.\n\n", TEST_PASSED);
    return 0;
}

int test_heap_logging()
{
    printf("=============> Running test_heap_logging...\n");
    slog_init("file_test_log", SLOG_FLAGS_ALL, 0);

    slog_config_t config;
    slog_config_get(&config);
    config.nUseHeap = 1;
    slog_config_set(&config);

    slog_info("Logging with heap memory");
    slog_destroy();

    printf("=============> test_heap_logging %s.\n\n", TEST_PASSED);
    return 0;
}

void *slog_test_thread(void *arg)
{
    slog_info((char *)arg);
    return NULL;
}

int test_thread_safe_logging()
{
    printf("=============> Running test_thread_safe_logging...\n");
    slog_init("lock_test_log", SLOG_FLAGS_ALL, 1);

    pthread_t thread1, thread2;
    int status = 0;

    status = pthread_create(&thread1, NULL, (void *)slog_test_thread, "Logging with thread safety enabled, thread 1");
    if (status != 0)
    {
        printf("=============> test_thread_safe_logging %s.\n\n", TEST_FAILED);
        return 1;
    }

    status = pthread_create(&thread2, NULL, (void *)slog_test_thread, "Logging with thread safety enabled, thread 2");
    if (status != 0)
    {
        printf("=============> test_thread_safe_logging %s.\n\n", TEST_FAILED);
        return 1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    slog_destroy();

    printf("=============> test_thread_safe_logging %s.\n\n", TEST_PASSED);
    return 0;
}

int main()
{
    int failed = 0;
    
    failed += test_slog_basic();
    failed += test_slog_file_logging();
    failed += test_slog_formatting();
    failed += test_heap_logging();
    failed += test_thread_safe_logging();

    if (failed > 0)
    {
        printf("=============> %d test(s) %s.\n", failed, TEST_FAILED);
        return 1;  // Fail the GitHub Actions pipeline
    }

    printf("=============> All tests %s successfully.\n", TEST_PASSED);
    return 0;  // Pass the GitHub Actions pipeline
}
