## slog Logging Library - 1.8 build 22
SLog is simple and thread safe logging library for C/C++. Software is written for educational purposes and is distributed in the hope that it will be useful for anyone interested in this field.

### Installation
Installation is possible with `Makefile`
```
git clone https://github.com/kala13x/slog.git
cd slog
make
sudo make install
```

### Usage
If you want to use slog in your C/C++ application, include `slog.h` header in your source file and link slog library with `-lslog` linker flag while compiling your project. See example directory for more informations.


### Simple API
At first you should initialize slog
```c
int nFlags = SLOG_NOTAG | SLOG_ERROR;
int nFlags |= SLOG_WARN | SLOG_FATAL;
slog_init("logfile", nFlags, 0);
```

 - First argument is file name where log will be saved.
 - Second argument is logging flags which are allowed to print. 
 - Third argument is thread safety flag *(1 enabled, 0 disabled)*.

If thread safety flag is greater than zero, function initializes mutex and evety other call of any slog function is protected by mutex locks.

With the above slog initialization example only errors, warnings and not tagged messages will be printed because there is no other flags activated.
You can also activate or deactivate any logging level after slog initialization with `slog_enable()` and `slog_disable()` functions.

```c
/* Enable all logging levels */
slog_enable(SLOG_FLAGS_ALL);

/* Disable trace level (trace logs will not be printed anymore) */
slog_disable(SLOG_TRACE);

/* Enable trace messages again */
slog_enable(SLOG_TRACE);

/* Disable all logging levels */
slog_disable(SLOG_FLAGS_ALL);
```

You must deinitialize slog only if the thread safety flag is greater than zero (nTdSafe > 0) while initialization.

```c
slog_destroy();
```
Function destrois the mutex attribute and sets thread safety flag to zero.

### Configuration

Since version 1.8.* config file is not supported anymore but there is a way to change configuration parameters of already initialized slog.

Parameter    | Type              | Default        | Description
-------------|-------------------|----------------|-------------------------------
sFileName    | char array        | "slog"         | Output file name for logs.
sFilePath    | char array        | "./"           | Output file path for logs.
eColorFormat | SLOG_COLOR_FMT_E  | SLOG_COLOR_TAG | Output color format control.
nTraceTid    | uint8_t           | 0 (disabled)   | Trace thread ID and display in output
nToScreen    | uint8_t           | 1              | Enable screen logging.
nToFile      | uint8_t           | 0 (disabled)   | Enable file logging.
nFlush       | uint8_t           | 0 (disabled)   | Flush stdout after screen log.
nFlags       | uint16_t          | 0 (no logs)    | Allowed log level flags.

Any of those parameters above can be changed at the runtime with the `slog_config_set()` function.

Example:
```c
SLogConfig slgCfg;

/* Setup configuration parameters */
slgCfg.eColorFormat = SLOG_COLOR_TAG;
strcpy(slgCfg.sFileName, "myproject");
strcpy(slgCfg.sFilePath, "./logs/");
slgCfg.nTraceTid = 1;
slgCfg.nToScreen = 1;
slgCfg.nToFile = 0;
slgCfg.nFlush = 1;
slgCfg.nFlags = SLOG_FLAGS_ALL;

/* Tread safe call to update slog configuration */
slog_config_set(&slgCfg);
```

If you want to change only few parameters without resetting other ones, you can thread safe read current working configuration and update only needed parameters.

```c
SLogConfig slgCfg;
slog_config_get(&slgCfg);

/* Update needed parameters */
slgCfg.nTraceTid = 1;
slgCfg.nToFile = 1;

/* Tread safe call to update slog configuration */
slog_config_set(&slgCfg);
```

### Logging flags
Slog has its logging flags to print something with status code.

- `SLOG_NOTAG`
- `SLOG_LIVE`
- `SLOG_INFO`
- `SLOG_WARN`
- `SLOG_DEBUG`
- `SLOG_TRACE`
- `SLOG_ERROR`
- `SLOG_FATAL`

### Print and log something
Here is an example on how use slog:
```c
slog("Simple test message");
```

Slog ends strings automatically with new line character `\n`. If you want to display output without adding new line character, you must use `slogwn()` function.
```c
slogwn("Simple test message without new line character");
```

You can use old way logging function with a bit more control of parameters */
```c
slog_print(SLOG_DEBUG, 0, "Simple test message without new line character");
```

 - First argument is a log level flag of current message.
 - Second argument is a flag to add new line character at the end of the output. 
 - Third argument is a formated string which will be displayed in the output.

*Output, taken from example directory:*

![alt tag](https://github.com/kala13x/slog/blob/master/example/slog.png)

#### UPDATE
From version 1.5 we provide a cleaner option to generate errors without the need to provide the flag parameter. 

We defined macros based on the warning flags.

- `slog()`
- `slogwn()`
- `slog_live()`
- `slog_info()`
- `slog_warn()`
- `slog_debug()`
- `slog_error()`
- `slog_trace()`
- `slog_fatal()`

Each macro takes a formated string. Format tags prototype follows the same rules as the C standard library function `printf()`.

Bellow we provide an example that logs a debug message:

```c
slog_debug("The %s contains between %d and %d billion stars and at least %d billion planets.  ", "Milky Way", 200, 400, 100);
```

In addition, we added the option to print the corresponding file name and line number where a slog macro was called. This rule follows the macros which relate to a critical or trace flag, and shown bellow:

- `slog_trace()`
- `slog_fatal()`

Basic example:
```c
/* Log and print something fatal. */
slog_trace("Trace message trows source location.");
```
With expected output to be:

    2017.01.22-19:03:17.03 - <trace> [example.c:71] Trace message trows source location.

You can also trace source location wothout any output message:

```c
slog_trace();
```
With expected output to be:

    2017.01.22-19:03:17.03 - <trace> [example.c:72]

### Version
`slog_version()` is a function which returns version of slog. If argument is 1, it returns only version and build number. Otherwise it returns full version such as Build number, build date and etc.

Usage:
```c
printf("slog Version: %s", slog_version(1));
```

Output will be something like that:
```
slog Version: 1.5 build 1 (Jan 22 2017)
```

### Output
Here is example output messages of slog
```
2020.12.13-19:41:41.27 - Simple message without anything
2020.12.13-19:41:41.27 - Simple message with our own new line character
2020.12.13-19:41:41.27 - <debug> Old way printed debug message with our own new line character
2020.12.13-19:41:41.27 - <error> Old way printed error message with auto new line character
2020.12.13-19:41:41.27 - <warn> Warning message without variable
2020.12.13-19:41:41.27 - <info> Info message with string variable: test string
2020.12.13-19:41:41.27 - <note> Note message with integer variable: 69
(15203) 2020.12.13-19:41:41.27 - <debug> Debug message with enabled thread id tracing
(15203) 2020.12.13-19:41:41.27 - <error> Error message with errno string: Success
(15203) 2020.12.13-19:41:41.27 - <debug> Debug message in the file with full line color enabled
(15203) 2020.12.13-19:41:41.27 - <trace> [example.c:95] Trace message throws source location
(15203) 2020.12.13-19:41:41.27 - <fatal> [example.c:98] Fatal message also throws source location
(15203) 2020.12.13-19:41:41.28 - <debug> Disabled output coloring
(15203) 2020.12.13-19:41:41.28 - <trace> [example.c:108] 
(15203) 2020.12.13-19:41:41.28 - <debug> Above we traced source location without output message
```
