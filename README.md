## Slog Logging Library
Advanced logging library for C/C++ which parses log level from config file and prints log if log level from config is equal or higher than level argument while printing with slog() function.

### Usage
If you want to use slog, you need just two files, slog.c and slog.h
You must include slog.h header file in your code and add slog in Makefile or link it manual.
There is Makefile example in this repository and you can see it and modify.

### Simple API
At first you must initialise slog
```
slog_init("filename", 1, 3);
```
First argument "filename" is a filename where logs will be saved.
File name will be generated from argument and also from system date.
Finally file name will be something like that:
```
filename-2015-04-02.log
```

Second argument is for enabling/disabling log to file.

Enable   | Disable
---------|---------
1        | 0

If 1 is given, logs will be saved in file, but it wont if argument is 2.

Third argument is maximum of log levels.

### Print and log something
There is an example how use slog. You can also see, compile and run example.c source file where is full functional examples of slog.
```
slog(0, "[LIVE] Test message with level 0");
```
First argument is log level and second is message to print and/or save. Slog ands strings automatically with \n.

### Version
slog_version() is a function which returns version information of slog.

Usage:
```
slog(0, "Slog Version: %s", slog_version());
```
Output will be something like that:
```
2015:04:02:28 - Slog Version: 0.2.1 Snapshot Build 14
```

### Output
Here is example output strings of slog
```
2015:04:02:56 - [LIVE] Test message with level 0
2015:04:02:56 - [LIVE] Test message with level 1
2015:04:02:56 - [DEBUG] Test message with level 2
2015:04:02:56 - [DEBUG] Test message with level 3
2015:04:02:56 - [LIVE] Test message with char argument: test string
2015:04:02:56 - [LIVE] Test message with int argument: 69
```
