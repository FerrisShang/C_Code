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

bool is_valid_num(char* s)
{
    if (strlen(s) > 2 && s[0] == '0' && (s[1] & 0xDF) == 'X') {
        char* p = s + 2;
        while (*p) {
            if (!isxdigit(*p++)) {
                return false;
            }
        }
    } else {
        if (*s == '-')s++;
        while (*s) {
            if (!isdigit(*s++)) {
                return false;
            }
        }
    }
    return true;
}

uint32_t str2u32(char* s)
{
    if (strlen(s) > 2 && s[0] == '0' && (s[1] & 0xDF) == 'X') {
        char* p = &s[2];
        uint32_t res = 0;
        while (*p) {
            res <<= 4;
            if (*p > '9') {
                res += (*p & 0xDF) - 'A' + 10;
            } else {
                res += *p - '0';
            }
            p++;
        }
        return res;
    } else {
        return atoi(s);
    }
}

