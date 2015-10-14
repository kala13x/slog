## slog Logging Library - 1.4 build 75
Slog is simple and thread safe logging library for C/C++. Software is written for educational purposes and is distributed in the hope that it will be useful for anyone interested in this field.

### Installation
Installation is possible with makefile
```
git clone https://github.com/kala13x/slog.git
cd slog/src
make
sudo make install
```

### Usage
If you want to use slog in your C/C++ application, include slog.h header in your source file and link slog library with -lslog linker flag while compiling your project. See examples in example.c and Makefile.


### Simple API
At first you must initialise slog
```
init_slog("filename", "slog.cfg", 3);
```
Function parses config file, reads log level and save to file flag from config. First argument is file name where log will be saved and second argument conf is config file path to be parsedand. Third argument is log level. If you will not initialize slog, it will only print messages with loglevel 0.

### Config file

Log level and log to file flags can be parsed from config file.

Example of config file:
```
LOGLEVEL 3
LOGTOFILE 1
PRETTYLOG 0
FILESTAMP 1
```
First value is log level to control which levels should printed.

Second is log to file flag

Enable   | Disable
---------|---------
1        | 0

If flag is 1, logs will be saved in file, otherwise it wont.

Third value is pretty log, if this flag is enabled, slog will save colored strings in file, otherwise it wont.

Fourth value is log file timstamp. If this flag is enabled, the log filename provided will have a datestamp and ".log" appended to it for each write. Otherwise log filename will always be the same as provided to init_slog().

### Logging flags
Slog has its logging flags to print something with status code.

- SLOG_LIVE
- SLOG_INFO
- SLOG_WARN
- SLOG_DEBUG
- SLOG_ERROR
- SLOG_FATAL
- SLOG_NONE

![alt tag](https://github.com/kala13x/slog/blob/master/slog.png)

### Print and log something
Here is an example how use slog.
```
slog(0, SLOG_LIVE, "Test message with level 0");
```
First argument is log level, second argument is logging flag, and third is message to print and/or save. Slog ends strings automatically with new line character \n.

### Colorize output
You can colorize strings with strclr function. Usage is very simple, first argument is color value and second argument is string which we want to colorize.

 Color values are:
- 0 - Normal
- 1 - Green
- 2 - Red
- 3 - Yellow
- 4 - Blue
- 5 - Nagenta
- 6 - Cyan
- 7 - White

For example, if we want to print something with red color, code will be something like that:
```
char *ret = strclr(2, "Test string");
slog(0, SLOG_NONE, "This is colorized string: %s", ret);
```

### Get date
You can check and get system date with get_slog_date function. Argument is pointer of SlogDate structure. For example if you want to print system date which uses slog, code will be something like that:
```
SlogDate date;
get_slog_date(&date);
```
Values will be saved with 24h format at SlogDate structure members.
```
date.year;
date.mon;
date.day;
date.hour;
date.min;
date.sec;
```

### Version
slog_version() is a function which returns version of slog. If argument is 1, it returns only version and build number. Otherwise it returns full version such as Build number, build name and etc.

Usage:
```
slog(0, SLOG_NONE, "slog Version: %s", slog_version(0));
```
Output will be something like that:
```
2015.05.01-15:59:58 - Slog Version: 0.2.3 Snapshot Build 16 (May  1 2015)
```

### Output
Here is example output strings of slog
```
2015.07.07-18:07:08 - [INFO]  Loading logger config from: slog.cfg
2015.07.07-18:07:08 - [LIVE]  Test message with level 0
2015.07.07-18:07:08 - [WARN]  Warn message with level 1
2015.07.07-18:07:08 - [INFO]  Info message with level 2
2015.07.07-18:07:08 - [LIVE]  Test message with level 3
2015.07.07-18:07:08 - [DEBUG] Debug message with char argument: test string
2015.07.07-18:07:08 - [ERROR] Error message with int argument: 69
2015.07.07-18:07:08 - [TEST]  This is our own colorized string

```
