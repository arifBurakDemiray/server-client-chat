// Arif Burak Demiray - 250201022
// contact - arifdemiray@std.iyte.edu.tr

#include "time_lib.h"
#include <stdio.h>
#include <stdlib.h>

void StrTime(char *timeStr)
{
    time_t Now;
    struct tm *Time;

    time(&Now);
    Time = localtime(&Now);

    sprintf(timeStr, "%04d-%02d-%02dT%02d:%02d:%02dZ", Time->tm_year + 1900,
            Time->tm_mon + 1, Time->tm_mday,
            Time->tm_hour, Time->tm_min, Time->tm_sec);
}