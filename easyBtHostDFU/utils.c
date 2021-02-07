#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"
#include "log.h"

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

char* get_datetime(void)
{
    static char buf[32];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buf, "%02d%02d%02d%02d%02d%02d", tm.tm_year % 100, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

uint32_t get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}

#include <sys/time.h>
#ifdef __linux__
#include <termios.h>
#else
#include <conio.h>
#endif
#include <unistd.h>
int get_num(int val_def)
{
    int value = 0, num = 0;
    while (1) {
        char c;
        {
#ifdef __linux__
            struct termios termios;
            tcgetattr(STDIN_FILENO, &termios);
            termios.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &termios);
            int ch = getchar();
            termios.c_lflag |= (ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &termios);
            c = ch;
#else
            c = getch();
#endif
        }
        if (c >= '0' && c <= '9' && value  < 200000000) {
            value = value * 10 + c - '0'; num++; putchar(c);
        } else if (c == 0x0A || c == 0x0D) {
            printf("\n");
            if (num) return value;
            else return val_def;
        } else if ((c == 0x7F || c == '\b') && num) {
            putchar('\b'); putchar(' '); putchar('\b'); num--; value /= 10;
        }
    }
}

char* get_file_dir(char* filepath)
{
    static char dir[128];
    strncpy(dir, filepath, 127);
    int i, len = strlen(dir);
    for (i = len - 1; i > 0; i--) {
        if (dir[i] == '/' || dir[i] == '\\') {
            dir[i + 1] = '\0';
            break;
        }
    }
    if (i == 0) {
        strcpy(dir, ".");
    }
    return dir;
}

void chdir_to_file(char* filepath)
{
    char* dir = get_file_dir(filepath);
    chdir(dir);
}

void dump_hex(uint8_t *data, int len)
{
    int i;
    for(i=0;i<len;i++){
        log("%02X ", data[i]);
    }
    log("\n");
}

char* hex_to_str(char* buf, uint8_t *data, int len)
{
    char *ret = buf;
    while(len--){
        *buf = (*data>>4)>9 ? (*data>>4)-10+'A' : (*data>>4)+'0'; buf++;
        *buf = (*data&15)>9 ? (*data&15)-10+'A' : (*data&15)+'0'; buf++;
        data++;
    }
    *buf = 0;
    return ret;
}

uint8_t* hexhex(uint8_t *hex, int hex_len, uint8_t *sub_hex, int sub_len)
{
    int i, j, found;
    for(i=0;i<=hex_len-sub_len && sub_len;i++){
        found = 1;
        for(j=0;j<sub_len;j++){
            if(hex[i+j] != sub_hex[j]){
                found = 0;
                break;
            }
        }
        if(found){ return &hex[i]; }
    }
    return NULL;
}

