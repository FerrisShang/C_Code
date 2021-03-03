#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdint.h>
#include <stdbool.h>

#define __WEAK __attribute__((weak))

char* strip(char* str, int* len);
int strcnt(const char* str, char* sub);
bool is_valid_num(const char* s);
uint32_t str2u32(const char* s);
uint32_t hex2uint(const uint8_t* d, int l);
int32_t hex2int(const uint8_t* d, int l);
char* uint2hexstr(char* buf, uint32_t hex, int byte_len);
char* hex2ms(char* buf, uint32_t hex, float base_ms);
void dump_hex(const void* p, int len);

#define MEM_STAT 0
#if MEM_STAT
void* __util_calloc (size_t __nmemb, size_t __size, char* line, int num);
void __util_free (void* __ptr, char* line, int num);
void util_mem_stat(void);
#define util_calloc(n,s) __util_calloc(n,s,(char*)__func__,__LINE__);
#define util_free(p) __util_free(p,(char*)__func__,__LINE__);
#else
#define util_calloc(n,s) calloc(n,s)
#define util_free(p) free(p)
#define util_mem_stat()
#endif

#endif /* __UTILS_H__ */
