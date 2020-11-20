#include "stdio.h"
#include "string.h"
#include "key_gen.h"
#include "uECC.h"
#include "assert.h"

uint8_t seed_priv_key[] = {
    0xF7, 0x8A, 0x86, 0x8F, 0x5E, 0x2C, 0x51, 0xD1,
    0xF4, 0x52, 0x85, 0x6E, 0x9C, 0x7D, 0x9A, 0xDF,
    0x76, 0x99, 0x53, 0x8D, 0xAD, 0xC0, 0xB8, 0xAE
};

bool generate_key(uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2])
{
    uint8_t res, priv_key[PRIV_KEY_SIZE], secret[PRIV_KEY_SIZE];
    assert(uECC_curve_private_key_size(ECC_TYPE) == PRIV_KEY_SIZE);
    res = uECC_make_key(plain_pub_key, priv_key, ECC_TYPE);
    if(!res){ return res; }
    res = uECC_shared_secret(plain_pub_key, seed_priv_key, secret, ECC_TYPE);
    if(!res){ return res; }
    for (int i = 0; i < PRIV_KEY_SIZE; i++) {
        cipher_priv_key[i] = priv_key[i] ^ secret[i];
    }
    return true;
}

bool write_pub_key_file(char *filename, uint8_t plain_pub_key[PRIV_KEY_SIZE*2])
{
    char buf[80];
    FILE *fp = fopen("public_key.c", "w");
    if(!fp){ return false; }
    fputs("#include <stdint.h>\n", fp);
    sprintf(buf, "// Corresponding to \"%s\"\n", filename);
    fputs(buf, fp);
    fputs("uint8_t dfu_public_key[] = {", fp);
    for(int i=0;i<PRIV_KEY_SIZE*2;i++){
        if(!(i%16)){ fputs("\n   ", fp); }
        sprintf(buf, " 0x%02X,", plain_pub_key[i]);
        fputs(buf, fp);
    }
    fputs("\n};\n", fp);
    fclose(fp);
    return true;
}

bool write_priv_key_file(char *filename, uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2])
{
    FILE *fp = fopen(filename, "wb");
    if(!fp){ return false; }
    fwrite(PRIV_KEY_MARK, sizeof(PRIV_KEY_MARK), 1, fp);
    fwrite(plain_pub_key, PRIV_KEY_SIZE*2, 1, fp);
    fwrite(cipher_priv_key, PRIV_KEY_SIZE, 1, fp);
    fclose(fp);
    if(!write_pub_key_file(filename, plain_pub_key)){
        return false;
    }
    return true;
}

bool check_key(uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2])
{
    uint8_t res, secret1[PRIV_KEY_SIZE], secret2[PRIV_KEY_SIZE], seed_pub_key[PRIV_KEY_SIZE*2];
    res = uECC_shared_secret(plain_pub_key, seed_priv_key, secret1, ECC_TYPE);
    if(!res){ return res; }
    for (int i = 0; i < PRIV_KEY_SIZE; i++) {
        cipher_priv_key[i] = cipher_priv_key[i] ^ secret1[i];
    }
    res = uECC_compute_public_key(seed_priv_key, seed_pub_key, ECC_TYPE);
    if(!res){ return res; }
    res = uECC_shared_secret(seed_pub_key, cipher_priv_key, secret2, ECC_TYPE);
    if(!res){ return res; }
    return !memcmp(secret1, secret2, PRIV_KEY_SIZE);
}

bool read_key_file(char *filename, uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2])
{
    FILE *fp = fopen(filename, "rb");
    if(!fp){ return false; }
    char str[sizeof(PRIV_KEY_MARK)];
    fread(str, sizeof(PRIV_KEY_MARK), 1, fp);
    if(memcmp(str, PRIV_KEY_MARK, sizeof(PRIV_KEY_MARK))){ return false; }
    fread(plain_pub_key, PRIV_KEY_SIZE*2, 1, fp);
    fread(cipher_priv_key, PRIV_KEY_SIZE, 1, fp);
    fclose(fp);
    return true;
}

