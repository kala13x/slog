## Slog Logging Library - 1.0.49
Slog is simple logging library for C/C++ which parses log level from config file and prints log if log level from config is equal or higher than argument level while printing with slog() function.

### Usage
If you want to use slog in your C/C++ application, at first compile it, then include slog.h header in your source file and link slog.a library while compiling your project. See examples in Makefile and example.c files.

### Simple API
At first you must initialise slog
```
slog_init("filename", "slog.cfg", 3);
```
Function parses config file, reads log level and save to file flag from config. First argument is file name where log will be saved and second argument conf is config file path to be parsedand. Third argument is log level.

### Config file

Log level and log to file flags can be parsed from config file.

Example of config file:
```
LOGLEVEL 3
LOGTOFILE 1
```
First value is log level to control which levels should printed.

Second is log to file flag

Enable   | Disable
---------|---------
1        | 0

If flag is 1, logs will be saved in file, otherwise it wont.


### Print and log something
Here is an example how use slog.
```
slog(0, "[LIVE] Test message with level 0");
```
First argument is log level and second is message to print and/or save. Slog ends strings automatically with new line character \n.

### Version
slog_version() is a function which returns version of slog. If argument is 1, it returns only version and build number. Otherwise it returns full version such as Build number, build name and etc.

Usage:
```
slog(0, "Slog Version: %s", slog_version(0));
```
Output will be something like that:
```
2015.05.01-15:59:58 - Slog Version: 0.2.3 Snapshot Build 16 (May  1 2015)
```

### Output
Here is example output strings of slog
```
2015.05.01-15:59:58 - [LIVE] Test message with level 0
2015.05.01-15:59:58 - [LIVE] Test message with level 1
2015.05.01-15:59:58 - [DEBUG] Test message with level 2
2015.05.01-15:59:58 - [DEBUG] Test message with level 3
2015.05.01-15:59:58 - [LIVE] Test message with char argument: test string
2015.05.01-15:59:58 - [LIVE] Test message with int argument: 69
```
