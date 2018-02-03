#include "Rtstring.h"


unsigned int atoi(char *str)
{
    char *p = str;
    unsigned int ret = 0;

    while ('\0' != p && *p != '\0')
    {
        if ('0' <= *p  && *p <= '9')
        {
            ret = ret * 10 + (*p - '0');
        }
        else
        {
            return 0;
        }
        p++;
    }

    return ret;
}
