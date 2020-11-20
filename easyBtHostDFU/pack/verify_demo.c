#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "uECC.h"
#include "sha256.h"
#include "key_gen.h"
#define print printf

#define DFU_CMDPKG_MARK          0x01BFDF55
#define MAX_IMG_NUM  (32)
#define MAX_BIN_SIZE (1<<24)
#define ECC_TYPE uECC_secp192r1()
#define PRIV_KEY_SIZE 24

void dump(void *p, int len)
{
	int i;
	for(i=0;i<len;i++){
		if(!(i%16)){ print("\n"); }
		print("%02X ", ((uint8_t*)p)[i]);
	}
	print("\n");
}
int get_file(char *filename, char **buf)
{
	if(!filename){
		*buf = NULL;
		return 0;
	}
#define MAX_FILE_SIZE (1<<24)
	*buf = calloc(1, MAX_FILE_SIZE);
	if(!*buf){
		print("Not enough memory\n");
        return 0;
	}
	memset(*buf, 0xFF, MAX_FILE_SIZE);
	FILE *fp = fopen(filename, "rb");
	if(!fp){
		print("File %s not found\n", filename);
        return 0;
	}
	uint32_t read_size = fread(*buf, 1, MAX_FILE_SIZE, fp);
	if(read_size == MAX_FILE_SIZE){
		print("File size exceed the limit size.\n");
        return 0;
	}
	read_size += (sizeof(uint32_t)-(read_size%sizeof(uint32_t)))%sizeof(uint32_t); // firmware must be word aligned
	return read_size;
}

int main(int argc, char *argv[]) // priv_key_file, cmd_file
{
    if(argc > 2){
        uint8_t cipher_priv_key[PRIV_KEY_SIZE], plain_pub_key[PRIV_KEY_SIZE*2];
        int res = read_key_file(argv[1], cipher_priv_key, plain_pub_key);
        if(!res){
            print("Read file \"%s\"failed. \n", argv[1]);
            exit(-1);
        }

        print("Public key:\n");
        dump(plain_pub_key, PRIV_KEY_SIZE * 2);

        uint8_t *cmd_data;
        uint32_t cmd_data_size = get_file(argv[2], (char**)&cmd_data);
        if(cmd_data_size < PRIV_KEY_SIZE*2){
            print("read %s failed or error format.\n", argv[2]);
            exit(-1);
        }
        dump(cmd_data, cmd_data_size);
        // cal hash of images
        uint8_t hash[SHA256_BLOCK_SIZE];
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, (const BYTE*)cmd_data, cmd_data_size - PRIV_KEY_SIZE*2);
        sha256_final(&ctx, hash);
        res = uECC_verify(plain_pub_key, hash, SHA256_BLOCK_SIZE,
                cmd_data + cmd_data_size - PRIV_KEY_SIZE*2, ECC_TYPE);
        print("uECC_verify=%d\n", res);
        exit(0);
    }
    printf("%s private_key_file cmd_data_file\n", argv[0]);
}
