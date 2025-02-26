#include <string.h>

#include "std.h"
#include "util.h"

int strEndsWith(const char *str, const char *sfx) 
{
    const char *p = str + strlen(str) - strlen(sfx);

    return strcmp(p, sfx) == 0;
}