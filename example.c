/*-----------------------------------------------------
Name: log.h
Date: 2015.04.01
Auth: kala13x (a.k.a 7th Ghost)
Desc: Header file of logging system
-----------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "slog.h"


/*---------------------------------------------
| Main function
---------------------------------------------*/
int main(int argc, char *argv[])
{
    /* Used variables */
    LogValues val;
    char char_arg[32];
    int int_arg;

    /* Init args */
    strcpy(char_arg, "test string");
    int_arg = 69;

    /* Initialise slog 
    * First argument is slog structure variables 
    * Second argument is log filename 
    * Third argument is log level */
    init_slog(&val, "example", 3);

    /* Log and print something with level 0 */
    slog(0, "[LIVE] Test message with level 0");

    /* Log and print something with level 1 */
    slog(1, "[LIVE] Test message with level 1");

    /* Log and print something with level 2 */
    slog(2, "[DEBUG] Test message with level 2");

    /* Log and print something with level 3 */
    slog(3, "[DEBUG] Test message with level 3");

    /* Log and print something with char argument */
    slog(0, "[LIVE] Test message with char argument: %s", char_arg);

    /* Log and print something with int argument */
    slog(0, "[LIVE] Test message with int argument: %d", int_arg);

    return 0;
}
