#include "stdio.h"
#include "assert.h"
#include "stdint.h"
#include "stdlib.h"
#include "utils.h"
#include "basic_type.h"

const char* enum_str_cb(int key, void *p)
{
    static char buf[100];
    if(key < 32768){
        sprintf(buf, "[enum: key:0x%X]", key);
        return buf;
    }else{
        return NULL;
    }
}
int main(void)
{
    int i;
    int32_t num;
    for(i=0;i<2;i++){
        num = ((rand()&0xFFFFFF)-0x800000);
        printf("%X %d %d 0x%X\n", num, num,
                hex2int((uint8_t*)&num, 3), hex2int((uint8_t*)&num, 3));
        assert(num == hex2int((uint8_t*)&num, 3));
    }

    char **out_list;
    int len;
    output_get(BTYPE_UNSIGNED, (uint8_t*)"\x01\x02", 2*8,
        &out_list, &len, enum_str_cb, NULL);
    for(i=0;i<len;i++){
        puts(out_list[i]);
    }
    output_get(BTYPE_ENUM, (uint8_t*)"\x01\x02", 2*8,
        &out_list, &len, enum_str_cb, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    output_get(BTYPE_ENUM, (uint8_t*)"\x01\x82", 2*8,
        &out_list, &len, enum_str_cb, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    puts("BTYPE_STREAM");
    output_get(BTYPE_STREAM, (uint8_t*)"\x01\x82\x01\x82\x01\x82\x01\x82", 8*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    puts("BTYPE_ADDRESS");
    output_get(BTYPE_ADDRESS, (uint8_t*)"\x01\x82\x01\x82\x01\x82\x01\x82", 8*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    puts("BTYPE_HEX");
    output_get(BTYPE_HEX, (uint8_t*)"\x01\x82\x01\x55", 10*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    puts("BTYPE_UNSIGNED");
    output_get(BTYPE_UNSIGNED, (uint8_t*)"\x01\x00\x01", 3*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    puts("BTYPE_SIGNED");
    output_get(BTYPE_SIGNED, (uint8_t*)"\x01\x00\x01", 3*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    output_get(BTYPE_T_0_625MS, (uint8_t*)"\x82\x38\x01", 3*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    output_get(BTYPE_T_1_25MS, (uint8_t*)"\x39\x01\x01", 3*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }
    output_get(BTYPE_T_10MS, (uint8_t*)"\x81\x01", 3*8,
        &out_list, &len, NULL, NULL);
    for(i=0;i<len;i++){ puts(out_list[i]); }

}

