#include "bt_encrypt.h"

/* ALG based on https://github.com/traviscross/bgaes */
#include "alg/aes.h"
#include "alg/cmac.h"


#define bxor(data, in_out, len) do{int l=len; while(l--){ in_out[l] ^= data[l]; } }while(0)
#define brev(data, len) do{int l=len/2; while(l--){ uint8_t t = data[l]; data[l] = data[len-l-1]; data[len-l-1] = t; }}while(0)

void btc_e(uint8_t *key, uint8_t *in_out)
{
	aes_encrypt_ctx cx;
	aes_encrypt_key128((unsigned char*)key, &cx);
	aes_encrypt((const unsigned char*)in_out, (unsigned char*)in_out, (const aes_encrypt_ctx*)&cx);
}

void btc_ah(uint8_t *irk, uint8_t *rand, uint8_t *out)
{
	uint8_t res[16], in[16] = {0};
	aes_encrypt_ctx cx;
	memcpy(&in[13], rand, 3);
	aes_encrypt_key128((unsigned char*)irk, &cx);
	aes_encrypt((const unsigned char*)in, (unsigned char*)res, (const aes_encrypt_ctx*)&cx);
	memcpy(out, &res[13], 3);
}

void btc_s1(uint8_t *k, uint8_t *r1, uint8_t *r2, uint8_t *out)
{
	int i;
	static uint8_t key[16];
	for(i=0;i<8;i++){
		out[i+0] = r1[8-1-i];
		out[i+8] = r2[8-1-i];
	}
	for(i=0;i<16;i++){ key[i] = k[16-1-i]; }
	btc_e(key, out);
	brev(out, 16);
}

void btc_c1(uint8_t *k, uint8_t *r, uint8_t *preq, uint8_t *pres,
		uint8_t iat, uint8_t rat, uint8_t *ia, uint8_t *ra, uint8_t *out)
{
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
	btc_e(key, out);
	brev(out, 16);
}

void btc_confirm_value(uint8_t *tk, uint8_t *rand, uint8_t *req_cmd, uint8_t *rep_cmd,
		uint8_t init_dev_addr_type, uint8_t *init_dev_addr,
		uint8_t rsp_dev_addr_type, uint8_t *rsp_dev_addr, uint8_t *out)
{
	btc_c1(tk, rand, req_cmd, rep_cmd, init_dev_addr_type,
			rsp_dev_addr_type, init_dev_addr, rsp_dev_addr, out);
}

void btc_aes_cmac(const uint8_t *k, uint8_t *data, int len, uint8_t *out)
{
	cmac_ctx ctx;
	cmac_init((const unsigned char*)k, 16, &ctx);
	cmac_data(data, len, &ctx);
	cmac_end(out, &ctx);
}

void btc_f4(uint8_t *u, uint8_t *v, uint8_t *x, uint8_t z, uint8_t *out)
{
	uint8_t data[65];
	memcpy(&data[0], u, 32);
	memcpy(&data[32], v, 32);
	data[64] = z;
	btc_aes_cmac(x, data, 65, out);
}

void btc_f5(uint8_t *w, uint8_t *n1, uint8_t *n2, uint8_t *a1, uint8_t *a2, uint8_t *out)
{
	const uint8_t salt[] = {
		0x6C,0x88, 0x83,0x91,0xAA,0xF5,0xA5,0x38,0x60,0x37,0x0B,0xDB,0x5A,0x60,0x83,0xBE
	};
	uint8_t key[16];
	uint8_t m[53] = {0, 0x62, 0x74, 0x6c, 0x65};
	btc_aes_cmac(salt, w, 32, key);
	memcpy(&m[5], n1, 16); memcpy(&m[21], n2, 16);
	memcpy(&m[37], a1, 7); memcpy(&m[44], a2, 7); m[51] = 0x01;
	btc_aes_cmac(key, m, 53, &out[0]);
	m[0] = 1;
	btc_aes_cmac(key, m, 53, &out[16]);
}

void btc_f6(uint8_t *w, uint8_t *n1, uint8_t *n2, uint8_t *r,
		uint8_t *iocap, uint8_t *a1, uint8_t *a2, uint8_t *out)
{
	uint8_t m[65];
	memcpy(&m[0], n1, 16); memcpy(&m[16], n2, 16); memcpy(&m[32], r, 16);
	memcpy(&m[48], iocap, 3); memcpy(&m[51], a1, 7); memcpy(&m[58], a2, 7);
	btc_aes_cmac(w, m, 65, out);
}

uint32_t btc_g2(uint8_t *u, uint8_t *v, uint8_t *x, uint8_t *y)
{
	uint8_t m[96], out[32];
	memcpy(&m[0], u, 32); memcpy(&m[32], v, 32); memcpy(&m[64], y, 16);
	btc_aes_cmac(x, m, 80, out);
	return (out[12]<<24) + (out[13]<<16) + (out[14]<<8) + (out[15]<<0);
}

void btc_h6(const uint8_t *w, uint8_t *keyId, uint8_t *out)
{
	btc_aes_cmac(w, keyId, 4, out);
}

void btc_h7(const uint8_t *salt, uint8_t *w, uint8_t *out)
{
	btc_aes_cmac(salt, w, 16, out);
}

