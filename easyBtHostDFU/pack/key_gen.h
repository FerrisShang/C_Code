#ifndef __KEY_GEN_H__
#define __KEY_GEN_H__

/********* Private Key file format **************+
| Plain  MARK(Onmicro DFU Private Key)  24 Bytes |
+------------------------------------------------+
|       Plain  Vender Public Key        48 Bytes |
|                                                |
+------------------------------------------------+
|       Cipher Vender Private Key       24 Bytes |
+------------------------------------------------+
| Cipher  Mark(Onmicro DFU Private Key) 24 Bytes |
+-----------------------------------------------*/

#include "stdint.h"
#include "stdbool.h"

#define PRIV_KEY_MARK "Onmicro DFU Private Key"
#define ECC_TYPE uECC_secp192r1()
#define PRIV_KEY_SIZE 24

bool generate_key(uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2]);
bool write_pub_key_file(char *filename, uint8_t plain_pub_key[PRIV_KEY_SIZE*2]);
bool write_priv_key_file(char *filename, uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2]);
bool check_key(uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2]);
bool read_key_file(char *filename, uint8_t cipher_priv_key[PRIV_KEY_SIZE], uint8_t plain_pub_key[PRIV_KEY_SIZE*2]);

#endif /*__KEY_GEN_H__*/
