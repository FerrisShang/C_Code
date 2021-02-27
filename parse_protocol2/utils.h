#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdint.h>
#include <stdbool.h>

#define __WEAK __attribute__((weak))

char* strip(char* str, int* len);
int strcnt(char* str, char* sub);
bool is_valid_num(char* s);
uint32_t str2u32(char* s);

#endif /* __UTILS_H__ */
