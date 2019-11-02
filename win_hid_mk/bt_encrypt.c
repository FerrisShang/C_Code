#ifndef __BT_ENCRYPT_H__
#define __BT_ENCRYPT_H__

#include "stdio.h"
#include "string.h"
#include "aes.h"

#define bxor(data, in_out, len) do{int l=len; while(l--){ in_out[l] ^= data[l]; } }while(0)
#define brev(data, len) do{int l=len/2; while(l--){ uint8_t t = data[l]; data[l] = data[len-l-1]; data[len-l-1] = t; }}while(0)

void btc_e(uint8_t *key, uint8_t *in_out)
{
	struct AES_ctx ctx;
	AES_init_ctx(&ctx, key);
	AES_ECB_encrypt(&ctx, in_out);
}

void btc_s1(uint8_t *key, uint8_t *r1, uint8_t *r2, uint8_t *out)
{
	int i;
	for(i=0;i<8;i++){
		out[i+0] = r1[8-1-i];
		out[i+8] = r2[8-1-i];
	}
	btc_e(key, out);
	brev(out, 16);
}

void btc_c1(uint8_t *k, uint8_t *r, uint8_t *preq, uint8_t *pres, uint8_t iat, uint8_t rat, uint8_t *ia, uint8_t *ra, uint8_t *out)
{
	static uint8_t p1[16];
	static uint8_t p2[16];
	static uint8_t key[16];
	int i;
	for(i=0;i<7;i++){
		out[i+0] = pres[7-1-i];
		out[i+7] = preq[7-1-i];
	}
	out[14] = rat; out[15] = iat;
	for(i=0;i<16;i++){ out[i] ^= r[16-1-i]; }
	for(i=0;i<16;i++){ key[i] = k[16-1-i]; }
	btc_e(key, out);
	for(i=0;i<6;i++){
		p2[i+4]  = ia[6-1-i];
		p2[i+10] = ra[6-1-i];
	}
	bxor(p2, out, 16);
	btc_e(k, out);
	brev(out, 16);
}

#define btc_confirm_value(tk, rand, req_cmd, rep_cmd, init_dev_addr_type, init_dev_addr, rsp_dev_addr_type, rsp_dev_addr, out) \
		btc_c1(tk, rand, req_cmd, rep_cmd, init_dev_addr_type, rsp_dev_addr_type, init_dev_addr, rsp_dev_addr, out);

void test(void){

	uint8_t out[16];
	uint8_t tk[16] = {0};
	uint8_t rand[] = {0xF5, 0xCE, 0xFC, 0xDF, 0xA4, 0xDB, 0x2B, 0x5B, 0x0F, 0xC0, 0x3A, 0x94, 0x47, 0x9A, 0xA2, 0x49};
	uint8_t req[] = {0x01, 0x04, 0x00, 0x01, 0x10, 0x03, 0x03};
	uint8_t resp[] = {0x02, 0x03, 0x00, 0x01, 0x10, 0x01, 0x01};
	uint8_t iat = 1;
	uint8_t rat = 0;
	uint8_t ia[] = {0x8F, 0xE1, 0x2D, 0x7D, 0xA8, 0x46};
	uint8_t ra[] = {0x78, 0x28, 0x00, 0x86, 0x19, 0x00};

	btc_confirm_value(tk, rand, req, resp, iat, ia, rat, ra, out);
	int i; for(i=0;i<16;i++)printf("%02X ", out[i]); printf("\n"); /* 09 2A B4 A8 C1 B8 8F 3A 6E 97 9A 87 55 C3 18 0B */

}

#endif /* __BT_ENCRYPT_H__ */

