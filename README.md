## slog Logging Library - 1.5 build 1
Slog is simple and thread safe logging library for C/C++. Software is written for educational purposes and is distributed in the hope that it will be useful for anyone interested in this field.

### Installation
Installation is possible with makefile
```
git clone https://github.com/kala13x/slog.git
cd slog/src
make
sudo make install
```

On Darwin/Apple Platform compile with
```
make -f Makefile.darwin
```

### Usage
If you want to use slog in your C/C++ application, include `slog.h` header in your source file and link slog library with `-lslog` linker flag while compiling your project. See examples directory for more informations.


### Simple API
At first you should initialize slog
```c
slog_init("logfile", "slog.cfg", 1, 3, 1);
```

 - First argument is file name where log will be saved. 
 - Second argument is config file path to be parsed *(see the next section for more informations about the config file)*. 
 - Third argument is max log level, if you will not initialize slog, it will only print messages with log level 0. 
 - Fourth argument is max log to file level, which is the same as the previous flag, with the difference that this corresponds to file log level. 
 - Fifth argument is thread safety flag *(1 enabled, 0 disabled)*. We recommend to always enable this flag.

**Note:** Although, you can run slog without initialization, we recommend this action. If you don't initialize slog, you'll only be able to export log messages in console. Also the default config values will be used.


### Config file

More configuration options can be parsed from config file.

If the confing file is `NULL`, the default values are set.
Values from the config file override the defaults.

Example of config file:
```
LOGLEVEL 1
LOGTOFILE 1
PRETTYLOG 0
FILESTAMP 1
LOGFILELEVEL 3
```
Flag         | Default | Internal Name  | Description
-------------|---------|----------------|------------
LOGLEVEL     | lvl*    | slg.level      | Max level to print to `stdout`.
LOGTOFILE    | 0       | slg.to_file    | If 0 will not write to file.
PRETTYLOG    | 0       | slg.pretty     | If 1 will output with color.
FILESTAMP    | 1       | slg.filestamp  | If 1 will add date to log name.
LOGFILELEVEL | lvl**   | flvl           | slg.file_level | Level required to write to file.

**Note:** `LOGFILELEVEL` and `LOGLEVEL` are completely independent of each other.

**`LOGLEVEL` is passed as third argument in `slog_init()`.*   
**`LOGFILELEVEL` is passed as fourth argument in `slog_init()`.*

### Logging flags
Slog has its logging flags to print something with status code.

- `SLOG_FLIVE`
- `SLOG_FINFO`
- `SLOG_FWARN`
- `SLOG_FDEBUG`
- `SLOG_FERROR`
- `SLOG_FFATAL`
- `SLOG_FPANIC`
- `SLOG_FNONE`

### Print and log something
Here is an example on how use slog.
```c
slog(0, SLOG_LIVE, "Test message with level 0");
```
- First argument is log level.
- Second argument is logging flag.
- Third is message to print and/or save. Slog ends strings automatically with new line character `\n`.

*Output, taken from example directory:*

![alt tag](https://github.com/GeorgeGkas/slog/blob/master/slog.png)

#### UPDATE
From version 1.5 we provide a cleaner option to generate errors without the need to provide the flag parameter. 

We defined macros based on the warning flags.

- `slog_none()`
- `slog_live()`
- `slog_info()`
- `slog_warn()`
- `slog_debug()`
- `slog_error()`
- `slog_fatal()`
- `slog_panic()`

Each macro take the following parameters:

 1. The log level.
 2. A formated string. Format tags prototype follows the same rules as the C standard library function `printf()`.
 3. Additional arguments. There should be the same number of these arguments as the number of `%-tags` that expect a value.

Bellow we provide an example that logs a debug message:

```c
slog_debug(0, "The %s contains between %d and %d billion stars and at least %d billion planets.  ", "Milky Way", 200, 400, 100);
```

In addition, we added the option to print the corresponding file name and line number where a slog macro was called. This rule follows the macros which relate to a critical flag, and shown bellow:

- `slog_warn()`
- `slog_error()`
- `slog_fatal()`
- `slog_panic()`

Basic example:
```c
/* Log and print something fatal. */
slog_fatal(0, "Fatal message. We fell into dead zone.");
```
With expected output to be:

    2017.01.22-19:03:17.03 - [FATAL] <example.c:71> -- Fatal message. We fell into dead zone.

### Colorize output
You can colorize strings with `strclr()` function. Usage is very simple, first argument is color value and second argument is string which we want to colorize.

Color values are:

- `CLR_NORMAL`
- `CLR_RED`
- `CLR_GREEN`
- `CLR_YELLOW`
- `CLR_BLUE`
- `CLR_NAGENTA`
- `CLR_CYAN`
- `CLR_WHITE`
- `CLR_RESET`

For example, if we want to print something with red color, code will be something like that:
```c
char *ret = strclr(CLR_NAGENTA, "Test string");
slog(0, SLOG_NONE, "This is colorized string: %s", ret);
```

### Get date
You can check and get system date with `get_slog_date()` function. Argument is pointer of SlogDate structure. For example if you want to print system date which uses slog, code will be something like that:
```c
SlogDate date;
slog_get_date(&date);
```
Values will be saved with 24h format at SlogDate structure members.
```c
date.year;
date.mon;
date.day;
date.hour;
date.min;
date.sec;
```

### Version
`slog_version()` is a function which returns version of slog. If argument is 1, it returns only version and build number. Otherwise it returns full version such as Build number, build name and etc.

Usage:
```c
printf("slog Version: %s", slog_version(0));
```
Output will be something like that:
```
slog Version: 1.5 build 1 (Jan 22 2017)
```

### Output
Here is example output strings of slog
```
2017.01.22-19:03:17.03 - [INFO] Loading logger config from: slog.cfg
2017.01.22-19:03:17.03 - [LIVE] Test message with level 0
2017.01.22-19:03:17.03 - [WARN] <example.c:56> -- Warn message with level 1
2017.01.22-19:03:17.03 - [INFO] Info message with level 2
2017.01.22-19:03:17.03 - [LIVE] Test message with level 3
2017.01.22-19:03:17.03 - [DEBUG] Debug message with char argument: test string
2017.01.22-19:03:17.03 - [ERROR] <example.c:68> -- Error message with int argument: 69
2017.01.22-19:03:17.03 - [FATAL] <example.c:71> -- Fatal message. We fell into dead zone.
2017.01.22-19:03:17.03 - [PANIC] <example.c:74> -- Panic here! We don't have tim....
2017.01.22-19:03:17.03 - [TEST] This is our own colorized string

```
