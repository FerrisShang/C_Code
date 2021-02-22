#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "utils.h"

char* strip(char* str, int* len)
{
    char* p = str, *end = str + strlen(str);
    if (!str) {
        return NULL;
    }
    while (*p && !isgraph(*p)) {
        p++;
    }
    while (end > p && !isgraph(*(end - 1))) {
        end--;
    }
    *end = '\0';
    if (len) {
        *len = end - p;
    }
    return p;
}

int strcnt(char* str, char* sub)
{
    int cnt = 0;
    if (!str) {
        return cnt;
    }
    do {
        char* p = strstr(str, sub);
        if (p) {
            cnt++;
        }
        str = p ? p + 1 : p;
    } while (str && *str);
    return cnt;
}

#ifndef __STRDUP__
#define __STRDUP__
char* __strdup (const char* s)
{
    size_t slen = strlen(s);
    char* result = (char*)malloc(slen + 1);
    if (result == NULL) {
        return NULL;
    }

    memcpy(result, s, slen + 1);
    return result;
}
#endif /* __STRDUP__ */

