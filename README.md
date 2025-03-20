[![MIT License](https://img.shields.io/badge/License-MIT-brightgreen.svg?)](https://github.com/kala13x/slog/blob/master/LICENSE)
[![Linux](https://github.com/kala13x/slog/actions/workflows/linux.yml/badge.svg)](https://github.com/kala13x/slog/actions/workflows/linux.yml)
[![MacOS](https://github.com/kala13x/slog/actions/workflows/macos.yml/badge.svg)](https://github.com/kala13x/slog/actions/workflows/macos.yml)
[![Windows](https://github.com/kala13x/slog/actions/workflows/windows.yml/badge.svg)](https://github.com/kala13x/slog/actions/workflows/windows.yml)
[![Valgrind](https://github.com/kala13x/slog/actions/workflows/tests.yml/badge.svg)](https://github.com/kala13x/slog/actions/workflows/tests.yml)
[![CodeQL](https://github.com/kala13x/slog/actions/workflows/codeql.yml/badge.svg)](https://github.com/kala13x/slog/actions/workflows/codeql.yml)

## SLOG - Logging Library
`slog` is a cross-platform and thread-safe logging library for C/C++ that offers rich functionality, including the ability to easily control verbosity levels, tag and colorize output, log to files, change configuration parameters on the fly, and provide an optional log callback for log collection, among many other features. Despite its extensive capabilities, the primary goal of `slog` is to remain as stable as possible while delivering maximum performance. For more details, refer to the `src/slog.h` file to check the release version you are using.

### Installation
Installation is possible with `CMake` for all platforms.
```bash
git clone https://github.com/kala13x/slog.git && cd slog
mkdir build && cd build
cmake .. && make
sudo make install
```

If for some reason `CMake` is not available on your system, you can try the `Makefile` that comes with the project (Linux only):
```bash
git clone https://github.com/kala13x/slog.git
cd slog && make && sudo make install
```

### Usage
If you want to use slog in your C/C++ application, include `slog.h` header in your source file and link the slog library with `-lslog` linker flag while compiling your project. See the example directory for more information.

### Logging flags
SLog has logging flags to control log levels and to display messages with tagged and colorized output.

- `SLOG_NOTAG`
- `SLOG_LIVE`
- `SLOG_INFO`
- `SLOG_WARN`
- `SLOG_DEBUG`
- `SLOG_TRACE`
- `SLOG_ERROR`
- `SLOG_FATAL`

### Simple API
At first, you should initialize slog:
```c
int nEnabledLevels = SLOG_NOTAG | SLOG_ERROR;
nEnabledLevels |= SLOG_WARN | SLOG_FATAL;

/* Setting SLOG_FLAGS_ALL will activate all logging levels */
// nEnabledLevels = SLOG_FLAGS_ALL;

slog_init("logfile", nEnabledLevels, 0);
```

 - The first argument is the name of the file where we want to save logs.
 - The second argument is the logging level flags which are allowed to print. 
 - Third argument is a thread safety flag *(1 enabled, 0 disabled)*.

If the thread safety flag is greater than zero, the function initializes mutex and every other call of any slog function is protected by lock.

With the above slog initialization example only errors, warnings, and not tagged messages will be displayed because there are no other flags activated during initialization. Any logging level can also be activated or deactivated after slog initialization by setting the flag with the `slog_enable()` and `slog_disable()` functions.

```c
/* Enable all logging levels */
slog_enable(SLOG_FLAGS_ALL);

/* Disable trace level (trace logs will not be displayed anymore) */
slog_disable(SLOG_TRACE);

/* Enable trace messages again */
slog_enable(SLOG_TRACE);

/* Disable all logging levels */
slog_disable(SLOG_FLAGS_ALL);
```

Deinitialization is needed only if the `nTdSafe` and/or `nKeepOpen` flags are greater than zero.
```c
slog_destroy();
```
Function destroys the mutex context, closes the output file, and resets the thread safety flag to zero.


### Print and log something in the file
Here is an example of how to use slog:
```c
slog("Simple message");
```

You can use old way logging function with a bit more control of parameters
```c
slog_display(SLOG_DEBUG, 0, "Simple debug message without new line character");
```

 - The first argument is a log-level flag of the current message.
 - The second argument is the flag to add a new line character at the end of the output *(1 add, 0 don't add)*.
 - The third argument is the formatted string which we want to display in the output.

#### Macros
SLog has the cleaner option to log messages without the need to provide the flag parameter. 

Here are defined macros based on the logging levels.

- `slog()`
- `slog_live()`
- `slog_info()`
- `slog_warn()`
- `slog_debug()`
- `slog_error()`
- `slog_trace()`
- `slog_fatal()`

Even shorter macros:

- `slogl()` same as `slog_live()`
- `slogi()` same as `slog_info()`
- `slogw()` same as `slog_warn()`
- `slogd()` same as `slog_debug()`
- `sloge()` same as `slog_error()`
- `slogt()` same as `slog_trace()`
- `slogf()` same as `slog_fatal()`

Each macro takes a formatted string and has another definition that does the same without newline characters.
- Format tags prototype follows the same rules as the C standard library function `printf()`.
- Definitions without new line characters begin with original names and end with the `_wn` string.

```c
slog("Simple message");
slog_wn("Simple message without new line character\n");

slogd("Debug message")
slogd_wn("Debug message without new line character\n")

slog_debug_wn("Another debug message without new line character\n")
slog_debug("The %s contains between %d and %d billion stars and at least %d billion planets.", "Milky Way", 200, 400, 100);
```

In addition, there are several options to print the corresponding file name and line number where a slog macro was called. This rule follows the macros which relate to a fatal or trace flag, as shown bellow:

- `slog_trace()`
- `slog_fatal()`

Display message with trace tag and print source location:
```c
slog_trace("Trace message throws source location.");
```

With an expected output to be:
```
2017.01.22-19:03:17.03 - <trace> [example.c:71] Trace message throws source location.
```

It can also trace source location without any output message:
```c
slog_trace();
```

With an expected output to be:
```
2017.01.22-19:03:17.03 - <trace> [example.c:72]
```

*Output, taken from example directory:*

![alt tag](https://github.com/kala13x/slog/blob/master/screens/slog.png)

### Configuration

Variables of `slog_config_t` structure:
Parameter    | Type              | Default           | Description
-------------|-------------------|-------------------|---------------------------
sFileName    | char array        | "slog"            | Output file name for logs.
sFilePath    | char array        | "./"              | Output file path for logs.
sSeparator   | char array        | " "               | Separator between info and log.
logCallback  | slog_cb_t         | NULL              | Centralized log callback function.
pCallbackCtx | void*             | NULL              | User data pointer passed to log callback.
eColorFormat | slog_coloring_t   | SLOG_COLORING_TAG | Output coloring format control.
eDateControl | slog_date_ctrl_t  | SLOG_TIME_ONLY    | Time and date control in the log output.
nKeepOpen    | uint8_t           | 1 (enabled)       | Keep the file handle open for future writes.
nTraceTid    | uint8_t           | 0 (disabled)      | Trace thread ID and display in output.
nToScreen    | uint8_t           | 1 (enabled)       | Enable or disable screen logging.
nUseHeap     | uint8_t           | 0 (disabled)      | Use dynamic allocation for output.
nToFile      | uint8_t           | 0 (disabled)      | Enable or disable file logging.
nIndent      | uint8_t           | 0 (disabled)      | Enable or disable indentations.
nRotate      | uint8_t           | 1 (enabled)       | Create new log file for each day.
nFlush       | uint8_t           | 0 (disabled)      | Flush output file after log.
nFlags       | uint16_t          | 0 (no logs)       | Enabled log flags.

Any of those parameters above can be changed at runtime with the `slog_config_set()` function.

Example:
```c
slog_config_t slgCfg;

/* Setup configuration parameters */
slgCfg.eColorFormat = SLOG_COLORING_TAG;
slgCfg.eDateControl = SLOG_TIME_ONLY;
strcpy(slgCfg.sFileName, "myproject");
strcpy(slgCfg.sFilePath, "./logs/");
slgCfg.logCallback = NULL;
slgCfg.pCallbackCtx = NULL;
slgCfg.nKeepOpen = 1;
slgCfg.nTraceTid = 1;
slgCfg.nToScreen = 1;
slgCfg.nUseHeap = 0;
slgCfg.nToFile = 0;
slgCfg.nFlush = 1;
slgCfg.nFlags = SLOG_FLAGS_ALL;

/* Tread safe call to update slog configuration */
slog_config_set(&slgCfg);
```

If you want to change only a few parameters without resetting other ones, you can thread safely read the current working configuration and update only the needed parameters.

```c
slog_config_t slgCfg;
slog_config_get(&slgCfg);

/* Update needed parameters */
slgCfg.nTraceTid = 1;
slgCfg.nToFile = 1;

/* Tread safe call to update slog configuration */
slog_config_set(&slgCfg);
```

### Dynamic allocation
If the output message is larger than the slog default message limit (8196 bytes) there is a possibility to enable dynamic allocation and use the heap for output messages:
```c
slog_config_t slgCfg;
slog_config_get(&slgCfg);
slgCfg.nUseHeap = 1;
slog_config_set(&slgCfg);

slog_debug("Your too big output message here");
```

### Coloring
SLog also has a coloring control and the possibility to colorize whole lines, just tag or disable coloring.
```c
slog_config_t slgCfg;
slog_config_get(&slgCfg);

/* Colorize only tags */
slgCfg.eColorFormat = SLOG_COLORING_TAG;
slog_config_set(&slgCfg);
slog_debug("Message with colorized tag");

/* Colorize full line */
slgCfg.eColorFormat = SLOG_COLORING_FULL;
slog_config_set(&slgCfg);
slog_debug("Message with full line color");

/* Disable coloring at all */ 
slgCfg.eColorFormat = SLOG_COLORING_DISABLE;
slog_config_set(&slgCfg);
slog_debug("Message without coloring");
```

### Time and date
SLog allows you to control the time and date format in the log output.
```c
slog_config_t slgCfg;
slog_config_get(&slgCfg);

/* Disable time and date in output */
slgCfg.eDateControl = SLOG_TIME_DISABLE;
slog_config_set(&slgCfg);
slog_debug("Message without time and date");

/* Enable only time in output */
slgCfg.eDateControl = SLOG_TIME_ONLY;
slog_config_set(&slgCfg);
slog_debug("Message with time only");

/* Enable date + time in output */ 
slgCfg.eDateControl = SLOG_DATE_FULL;
slog_config_set(&slgCfg);
slog_debug("Message with time and date");
```

Example output:
```
<debug> Message without time and date
02:11:33.36 - <debug> Message with time only
2021.05.23-02:11:34.36 - <debug> Message with time and date
```

### Thread ID tracing
If you are looking for additional information about threads while debugging, you can trace thread IDs and display them in the output.

Here is an example:
```c
slog_config_t slgCfg;
slog_config_get(&slgCfg);
slgCfg.nTraceTid = 1;
slog_config_set(&slgCfg);

slog_debug("Message with thread id");
```

With an expected output to be:
```
(15203) 2017.01.22-19:03:17.03 - <debug> Message with thread id.
```
Where `15203` is a thread identifier from which the message was printed.


### Indentations
With an enabled indentation flag, slog will automatically adjust the spacing between the information and the message.

```c
slog_config_t slgCfg;
slog_config_get(&slgCfg);
slgCfg.nIndnets = 1;
slog_config_set(&slgCfg);
```

With indentations enabled:

![alt tag](https://github.com/kala13x/slog/blob/master/screens/indent.png)

Without indentations enabled:

![alt tag](https://github.com/kala13x/slog/blob/master/screens/no-indent.png)


### Callback
If you want to collect logs for any purpose, use a callback function pointer. If this pointer is set, any log will be passed to the function this pointer points to.
```c
int log_callback(const char *pLog, size_t nLength, slog_flag_t eFlag, void *pCtx)
{
    (void)nLength; // Log message length
    (void)eFlag; // Logging flag of this message
    (void)pCtx; // Optional pointer passed to the callback

    printf("%s", pLog);
    return 0;
}

int main()
{
    slog_init("logfile", SLOG_FLAGS_ALL, 0);

    slog_config_t slgCfg;
    slog_config_get(&slgCfg);
    slgCfg.logCallback = log_callback;
    slgCfg.pCallbackCtx = NULL; // Optional pointer passed to the callback
    slog_config_set(&slgCfg);

    slog("This message will be passed to the callback function");

    slog_destroy();
    return 0;
}
```

If you return `-1` from the callback function, the log will no longer be printed to the screen or written to a file by `slog`. If you return `0`, the log will not be written to the screen but still to a file (if nToFile > 1). If you return `1` the logger will normally continue its routine.

### Version
Get `slog` version with the function `slog_version()`. The argument `uint8_t nShort` is a flag to get a short or full string of the version (1 short, 0 full).

Usage:
```c
printf("slog version: %s", slog_version(0));
```

The output will be something like this:
```
slog version: 1.8 build 22 (Dec 14 2020)
```

There are also definitions that can be used to check the version without using the function.

- `SLOG_VERSION_MAJOR` - Major version of the library.
- `SLOG_VERSION_MINOR` - Minor version of the library.
- `SLOG_BUILD_NUMBER` - Build number.


### Output
Here is an example of the log file context created by Slog:
```
Simple message without anything
02:11:34.36 - Simple message with time only
02:11:34.36 - Simple message with our own new line character
02:11:34.36 - <debug> Old way printed debug message with our own new line character
02:11:34.36 - <error> Old way printed error message with auto new line character
02:11:34.36 - <warn> Warning message without variable
02:11:34.36 - <info> Info message with string variable: test string
02:11:34.36 - <note> Note message with integer variable: 69
(8180) 02:11:34.36 - <debug> Debug message with enabled thread id tracing
(8180) 02:11:34.36 - <error> Error message with errno string: Success
(8180) 02:11:34.36 - <debug> Debug message in the file with full line color enabled
(8180) 02:11:34.36 - <trace> [example.c:105] Trace message throws source location
(8180) 02:11:34.36 - <fatal> [example.c:108] Fatal message also throws source location
(8180) 2021.05.23-02:11:34.36 - <debug> Debug message with time and date
(8180) 2021.05.23-02:11:34.36 - <debug> Disabled output coloring
(8180) 2021.05.23-02:11:34.36 - <trace> [example.c:124] 
(8180) 2021.05.23-02:11:34.36 - <debug> Above we traced source location without output message

```
