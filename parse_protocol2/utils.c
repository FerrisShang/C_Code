#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
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

int strcnt(const char* str, char* sub)
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

bool is_valid_num(const char* s)
{
    if (strlen(s) > 2 && s[0] == '0' && (s[1] & 0xDF) == 'X') {
        const char* p = s + 2;
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

uint32_t str2u32(const char* s)
{
    if (strlen(s) > 2 && s[0] == '0' && (s[1] & 0xDF) == 'X') {
        const char* p = &s[2];
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

uint32_t hex2uint(const uint8_t* d, int l)
{
    uint32_t res = 0;
    int i;
    for (i = l - 1; i >= 0; i--) {
        res <<= 8;
        res += d[i];
    }
    return res;
}
int32_t hex2int(const uint8_t* d, int l)
{
    int i, sign = d[l - 1] & 0x80;
    int32_t res = (sign ? 0xFFFFFF80 : 0) | (d[l - 1] & 0x7F);
    for (i = l - 2; i >= 0; i--) {
        res <<= 8;
        res += d[i];
    }
    return res;
}
char* uint2hexstr(char* buf, uint32_t hex, int byte_len)
{
    char num[32], *p = num;
    sprintf(num, "%08X", hex);
    while (p && *p == '0' && *(p + 1) == '0' && *(p + 2) && (p - num) / 2 < 4 - byte_len) {
        p += 2;
    }
    sprintf(buf, "0x%s", p);
    return buf;
}
char* hex2ms(char* buf, uint32_t hex, float base_ms)
{
    int i;
    long long t_us = hex * base_ms * 1000.0;
    char t_str[32];
    sprintf(t_str, "%f", t_us / (t_us > 1000000 ? 1000000.0 : 1000.0));
    for (i = strlen(t_str) - 1; i >= 0 && strstr(t_str, "."); i--) {
        if (t_str[i] == '0' || t_str[i] == '.') {
            t_str[i] = '\0';
        } else {
            break;
        }
    }
    sprintf(buf, "%s%s (%d)", t_str, t_us > 1000000 ? "s" : "ms", hex);
    return buf;
}

void dump_hex(const void* p, int len)
{
    printf("dump: ");
    for (int i = 0; i < len; i++) {
        printf("%02X ", ((uint8_t*)p)[i]);
    }
    printf("\n");
}

#if MEM_STAT
#define MAX_MEM_REC 1<<24
static size_t mem_rec[MAX_MEM_REC];
void* __util_calloc (size_t __nmemb, size_t __size, char* line, int num)
{
    void* ret = calloc(__nmemb, __size);
    int i;
    for (i = 0; i < MAX_MEM_REC; i++) {
        if (mem_rec[i] == 0) {
            mem_rec[i] = (size_t)ret;
            break;
        }
    }
    if (0 && i == MAX_MEM_REC) {
        puts("no memory recored !");
        assert(i != MAX_MEM_REC);
    }
    return ret;
}
void __util_free (void* __ptr, char* line, int num)
{
    int i;
    for (i = 0; i < MAX_MEM_REC; i++) {
        if (mem_rec[i] == (size_t)__ptr) {
            mem_rec[i] = 0;
            break;
        }
    }
    if (i == MAX_MEM_REC) {
        printf("free invalid pointer! %s@%d\n", line, num);
        assert(i != MAX_MEM_REC);
    }
    free(__ptr);
}
void util_mem_stat(void)
{
    int i, cnt = 0;
    for (i = 0; i < MAX_MEM_REC; i++) {
        if (mem_rec[i]) {
            cnt++;
            //printf("%d, %p\n", i, (void*)mem_rec[i]);
        }
    }
    printf("mem no free:%d\n", cnt);
}
#endif

