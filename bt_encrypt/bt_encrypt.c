#include "bt_encrypt.h"
#include "stdlib.h"

/* ALG based on https://github.com/traviscross/bgaes */
#include "alg/aes.h"
#include "alg/cmac.h"
/* uEcc */
#include "alg/uECC.h"
/* ccm */
#include "alg/ccm.h"


#define bxor(data, in_out, len) do{int l=len; while(l--){ in_out[l] ^= data[l]; } }while(0)
#define brev(data, len) \
	do{int l=len/2;\
		while(l--){\
			uint8_t t = ((uint8_t*)data)[l];\
			((uint8_t*)data)[l] = ((uint8_t*)data)[len-l-1];\
			((uint8_t*)data)[len-l-1] = t;\
		}\
	}while(0)

/**************************************************************************
 * Bluetooth Cryptographic Toolbox, All variable is LITTLE ENDIAN!!
 *************************************************************************/

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
	memcpy(&out[0], r1, 8);
	memcpy(&out[8], r2, 8);
	btc_e(k, out);
}

void btc_c1(uint8_t *k, uint8_t *r, uint8_t *preq, uint8_t *pres,
		uint8_t iat, uint8_t rat, uint8_t *ia, uint8_t *ra, uint8_t *out)
{
	uint8_t p1[16] = {0};
	uint8_t p2[16] = {0};
	memcpy(&p1[0], pres, 7);
	memcpy(&p1[7], preq, 7);
	p1[14] = rat; p1[15] = iat;
	memcpy(out, r, 16);
	bxor(p1, out, 16);
	btc_e(k, out);
	for(int i=0;i<6;i++){
		memcpy(&p2[4], ia, 6);
		memcpy(&p2[10], ra, 6);
	}
	bxor(p2, out, 16);
	btc_e(k, out);
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

static int uECC_RNG(uint8_t *dest, unsigned size)
{
	for(int i=0;i<size;i++){ dest[i] = rand() & 0xFF; }
}


/**************************************************************************
 * Bluetooth Cryptographic Toolbox, All variable is BIG ENDIAN!!
 *************************************************************************/
void btc_hash(uint8_t *irk, uint8_t *rand, uint8_t *out)
{
	uint8_t _irk[16], _rand[16];
	for(int i=0;i<16;i++){ _irk[i] = irk[16-1-i]; _rand[i] = rand[16-1-i]; }
	btc_ah(_irk, _rand, out);
	brev(out, 3);
}
void btc_stk(uint8_t *tk, uint8_t *rrand, uint8_t *irand, uint8_t *out)
{
	uint8_t key[16];
	uint8_t r1[16], r2[16];
	for(int i=0;i<8;i++){
		r1[i] = rrand[8-1-i];
		r2[i] = irand[8-1-i];
	}
	for(int i=0;i<16;i++){ key[i] = tk[16-1-i]; }
	btc_s1(key, r1, r2, out);
	brev(out, 16);
}
void btc_legacy_confirm(uint8_t *tk, uint8_t *rand, uint8_t *preq, uint8_t *pres,
		uint8_t iat, uint8_t *ia, uint8_t rat, uint8_t *ra, uint8_t *out)
{
	int i;
	uint8_t _tk[16]; uint8_t _rand[16];
	uint8_t _preq[7]; uint8_t _pres[7];
	uint8_t _ia[6]; uint8_t _ra[6];
	for(i=0;i<16;i++){ _tk[i] = tk[16-i-1]; _rand[i] = rand[16-i-1]; }
	for(i=0;i<7;i++){ _preq[i] = preq[7-i-1]; _pres[i] = pres[7-i-1]; }
	for(i=0;i<6;i++){ _ia[i] = ia[6-i-1]; _ra[i] = ra[6-i-1]; }
	btc_c1(_tk, _rand, _preq, _pres, iat, rat, _ia, _ra, out);
	brev(out, 16);
}

void btc_sc_confirm(uint8_t *pk1, uint8_t *pk2, uint8_t *rand, uint8_t ri, uint8_t *out)
{
	uint8_t _pk1[32], _pk2[32], _rand[16];
	//for(int i=0;i<32;i++){ _pk1[32-i-1] = pk1[i]; _pk2[32-i-1] = pk2[i]; }
	for(int i=0;i<16;i++){ _rand[16-i-1] = rand[i]; }
	//btc_f4(_pk1, _pk2, _rand, ri, out);
	btc_f4(pk1, pk2, _rand, ri, out);
	brev(out, 16);
}

void btc_dhkey(uint8_t *pubKey, uint8_t *privKey, uint8_t *out)
{
	static uECC_Curve c = NULL;
	if(!c){
		c = uECC_secp256r1();
		uECC_set_rng(uECC_RNG);
	}
	uint8_t private_key[32];
	uint8_t public_key[64];
	for(int i=0;i<32;i++){ private_key[32-i-1] = privKey[i]; }
	for(int i=0;i<32;i++){ public_key[32-i-1] = pubKey[i]; }
	for(int i=0;i<32;i++){ public_key[64-i-1] = pubKey[32+i]; }
	uECC_shared_secret(public_key, private_key, out, c);
	brev(out, 32);
}

void btc_mackey_ltk(uint8_t *dhkey, uint8_t *irand, uint8_t *rrand, uint8_t *ia_t, uint8_t *ra_t, uint8_t *out)
{
	uint8_t _irand[16], _rrand[16], _ia[7], _ra[7];
	for(int i=0;i<16;i++){ _irand[16-i-1] = irand[i]; _rrand[16-i-1] = rrand[i]; }
	for(int i=0;i<7;i++){ _ia[7-i-1] = ia_t[i]; _ra[7-i-1] = ra_t[i]; }
	btc_f5(dhkey, _irand, _rrand, _ia, _ra, out);
	brev(&out[0], 16); brev(&out[16], 16);
}

void btc_dhkey_check(uint8_t *mackey, uint8_t *n1, uint8_t *n2, uint8_t *r,
		uint8_t *iocap, uint8_t *a1_t, uint8_t *a2_t, uint8_t *out)
{

	static uint8_t r0[16] = {0};
	uint8_t _mackey[16], _n1[16], _n2[16], _r[16], _iocap[3], _a1[7], _a2[7];
	if(!r) r = r0;
	for(int i=0;i<16;i++){
		_mackey[16-i-1] = mackey[i]; _r[16-i-1] = r[i];
		_n1[16-i-1] = n1[i]; _n2[16-i-1] = n2[i];
	}
	for(int i=0;i<3;i++){ _iocap[i] = iocap[3-i-1]; }
	for(int i=0;i<7;i++){ _a1[i] = a1_t[7-i-1]; _a2[i] = a2_t[7-i-1]; }
	btc_f6(_mackey, _n1, _n2, _r, _iocap, _a1, _a2, out);
	brev(out, 16);
}

void btc_ll_enc_ctx(uint8_t *SKDm, uint8_t *SKDs, uint8_t *LTK,
		uint8_t *IVm, uint8_t *IVs, btc_ll_enc_ctx_t *ctx)
{
	int8_t ltk[16];
	memcpy(ctx->IV, IVm, 4); memcpy(&ctx->IV[4], IVs, 4);
	for(int i=0;i<16;i++){ ltk[16-i-1] = LTK[i]; }
	for(int i=0;i<8;i++){ ctx->SK[16-i-1] = SKDm[i]; ctx->SK[8-i-1] = SKDs[i]; }
	btc_e((uint8_t*)ltk, ctx->SK);
	ccm_init_and_key(ctx->SK, 16, &ctx->ctx);
	memcpy(ctx->nonce + 5, ctx->IV, 8);
}

void btc_ll_encrypt(btc_ll_enc_ctx_t *ctx, const uint32_t counter, const uint8_t isMaster,
		uint8_t LLID, uint8_t *data, uint8_t len, uint8_t *out, uint8_t *mic)
{
	ctx->nonce[4] = isMaster?0x80:0x00;
	*(uint32_t*)ctx->nonce = counter;
	LLID &= 0x03;
	if(out){ memcpy(out, data, len); } else { out = data; }
	ccm_encrypt_message(ctx->nonce, 13, &LLID, 1, (unsigned char*)out, len,
			(unsigned char*)mic, 4, &ctx->ctx);
}

int btc_ll_decrypt(btc_ll_enc_ctx_t *ctx, uint8_t LLID, unsigned char *data,
		unsigned long len, uint8_t *out, uint8_t *mic)
{
	LLID &= 0x03;
	if(out){ memcpy(out, data, len); } else { out = data; }
	return ccm_decrypt_message(ctx->nonce, 13, &LLID, 1, (unsigned char*)out, len,
			(unsigned char*)mic, 4, &ctx->ctx);
}

uint32_t btc_crc24(uint32_t init_crc, uint8_t *buf, int len)
{
	static int init_flag, i;
	static uint32_t crc24_table[256];
	if(!init_flag) {//init_crc24()
		for (i = 0; i < 256; i++) {
			uint32_t _crc = 0;
			uint32_t c = i;
			for (int j = 0; j < 8; j++) {
				if ((_crc ^ c) & 0x1)
					//poly: 24,10,9,6,4,3,1,0
					_crc = (_crc >> 1) ^ 0xDA6000; // Polynomial, bit reverse of 0x00065B
				else
					_crc = _crc >> 1;
				c = c >> 1;
			}
			crc24_table[i] = _crc;
		}
		init_flag = 1;
	}
	// bit reverse
	uint32_t crc = 0;
	for(i=0;i<24;i++){
		crc = (crc << 1) | (init_crc & 1);
		init_crc >>= 1;
	}
	//crc = crc & 0xFFFFFF;
	for(i=0;i<len;i++) { crc = ((crc >> 8) ^ crc24_table[(crc ^ buf[i]) & 0xFF]); }
	return crc;
}

void btc_whitening(uint8_t* data, uint8_t len, uint8_t ch_idx)
{
	ch_idx = (ch_idx << 1) | 0x80;
	while (len--){
		for (uint8_t i = 1; i; i <<= 1){
			ch_idx >>= 1;
			if (ch_idx & 1){
				ch_idx ^= 0x88; // pos0 & pos4
				(*data) ^= i;
			}
		}
		data++;
	}
}

