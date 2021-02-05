#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdint.h>
#include <stdbool.h>

bool is_valid_num(char* s);
uint32_t str2u32(char* s);
char* get_datetime(void);
uint32_t get_time_ms(void);
int get_num(int val_def);
char* get_file_dir(char* filepath);
void chdir_to_file(char* filepath);
void dump_hex(uint8_t *data, int len);
char* hex_to_str(char* buf, uint8_t *data, int len);

#endif /* __UTILS_H__ */
