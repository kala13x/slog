## slog Logging Library - 1.4 build 85
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
If you want to use slog in your C/C++ application, include slog.h header in your source file and link slog library with `-lslog` linker flag while compiling your project. See examples in example.c and Makefile.


### Simple API
At first you must initialise slog
```c
slog_init("filename", "slog.cfg", 1, 3, 1);
```
Function parses config file, reads log level and save to file flag from config. First argument is file name where log will be saved. Second argument conf is config file path to be parsed. Third argument is log level, if you will not initialize slog, it will only print messages with log level 0. Fourth Argument is log to file level and fifth argument is thread safety flag (1 enabled, 0 disabled).


### Config file

More configuration options can be parsed from config file.

If the conf is `NULL` the default values are set.
Values from the config file override the defaults.

Example of config file:
```
LOGLEVEL 1
LOGTOFILE 1
PRETTYLOG 0
FILESTAMP 1
LOGFILELEVEL 3
```
Flag         | Type  | Default | Internal Name  | Description
-------------|-------|---------|----------------|------------
LOGLEVEL     | level | lvl     | slg.level      | Max level to print to stdout
LOGTOFILE    | bool  | 0       | slg.to_file    | If 0 will not write to file
PRETTYLOG    | bool  | 0       | slg.pretty     | If 1 will output with color
FILESTAMP    | bool  | 1       | slg.filestamp  | If 1 will add date to log name
LOGFILELEVEL | level | flvl    | slg.file_level | Level required to write to file

`LOGFILELEVEL` and `LOGLEVEL` are completely independent of each other.

### Logging flags
Slog has its logging flags to print something with status code.

- SLOG_LIVE
- SLOG_INFO
- SLOG_WARN
- SLOG_DEBUG
- SLOG_ERROR
- SLOG_FATAL
- SLOG_PANIC
- SLOG_NONE

![alt tag](https://github.com/GeorgeGkas/slog/blob/master/slog.png)

### Print and log something
Here is an example how use slog.
```c
slog(0, SLOG_LIVE, "Test message with level 0");
```
First argument is log level, second argument is logging flag, and third is message to print and/or save. Slog ends strings automatically with new line character `\n`.

### Colorize output
You can colorize strings with `strclr()` function. Usage is very simple, first argument is color value and second argument is string which we want to colorize.

Color values are:

- CLR_NORMAL
- CLR_RED
- CLR_GREEN
- CLR_YELLOW
- CLR_BLUE
- CLR_NAGENTA
- CLR_CYAN
- CLR_WHITE
- CLR_RESET

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
slog(0, SLOG_NONE, "slog Version: %s", slog_version(0));
```
Output will be something like that:
```
2017.01.21-23:13:06.99 - slog Version: 1.4 build 85 (Jan 21 2017)
```

### Output
Here is example output strings of slog
```
2017.01.21-23:13:06.99 - [INFO] Loading logger config from: slog.cfg
2017.01.21-23:13:06.99 - [LIVE] Test message with level 0
2017.01.21-23:13:06.99 - [WARN] Warn message with level 1
2017.01.21-23:13:06.99 - [INFO] Info message with level 2
2017.01.21-23:13:06.99 - [LIVE] Test message with level 3
2017.01.21-23:13:06.99 - [DEBUG] Debug message with char argument: test string
2017.01.21-23:13:06.99 - [ERROR] Error message with int argument: 69
2017.01.21-23:13:06.99 - [TEST] This is our own colorized string
```