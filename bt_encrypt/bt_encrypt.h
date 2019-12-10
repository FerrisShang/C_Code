#ifndef __BT_ENCRYPT_H__
#define __BT_ENCRYPT_H__
#include "stdint.h"

/**************************************************************************
 * Bluetooth Cryptographic Toolbox, All variable is LITTLE ENDIAN!!
 *************************************************************************/
void btc_e(uint8_t *key, uint8_t *in_out);
void btc_ah(uint8_t *irk, uint8_t *rand, uint8_t *out);
void btc_c1(uint8_t *k, uint8_t *r, uint8_t *preq, uint8_t *pres,
			uint8_t iat, uint8_t rat, uint8_t *ia, uint8_t *ra, uint8_t *out);
void btc_s1(uint8_t *k, uint8_t *r1, uint8_t *r2, uint8_t *out);
void btc_confirm_value(uint8_t *tk, uint8_t *rand, uint8_t *req_cmd, uint8_t *rep_cmd,
		uint8_t init_dev_addr_type, uint8_t *init_dev_addr,
		uint8_t rsp_dev_addr_type, uint8_t *rsp_dev_addr, uint8_t *out);
void btc_aes_cmac(const uint8_t *k, uint8_t *data, int len, uint8_t *out);
void btc_f4(uint8_t *u, uint8_t *v, uint8_t *x, uint8_t z, uint8_t *out);
void btc_f5(uint8_t *w, uint8_t *n1, uint8_t *n2, uint8_t *a1, uint8_t *a2, uint8_t *out);
void btc_f6(uint8_t *w, uint8_t *n1, uint8_t *n2, uint8_t *r,
		uint8_t *iocap, uint8_t *a1, uint8_t *a2, uint8_t *out);
uint32_t btc_g2(uint8_t *u, uint8_t *v, uint8_t *x, uint8_t *y);
void btc_h6(const uint8_t *w, uint8_t *keyId, uint8_t *out);
void btc_h7(const uint8_t *salt, uint8_t *w, uint8_t *out);

/**************************************************************************
 * Bluetooth Cryptographic Toolbox, All variable is BIG ENDIAN!!
 *************************************************************************/
void btc_hash(uint8_t *irk, uint8_t *rand, uint8_t *out);
void btc_stk(uint8_t *tk, uint8_t *rrand, uint8_t *irand, uint8_t *out);
void btc_legacy_confirm(uint8_t *tk, uint8_t *rand, uint8_t *preq, uint8_t *pres,
		uint8_t iat, uint8_t *ia, uint8_t rat, uint8_t *ra, uint8_t *out);
void btc_sc_confirm(uint8_t *pk1, uint8_t *pk2, uint8_t *rand, uint8_t ri, uint8_t *out);
void btc_dhkey(uint8_t *pubKey, uint8_t *privKey, uint8_t *out);
void btc_mackey_ltk(uint8_t *dhkey, uint8_t *irand, uint8_t *rrand, uint8_t *ia_t, uint8_t *ra_t, uint8_t *out);
void btc_dhkey_check(uint8_t *mackey, uint8_t *n1, uint8_t *n2, uint8_t *r,
		uint8_t *iocap, uint8_t *a1_t, uint8_t *a2_t, uint8_t *out);
#include "alg/ccm.h"
typedef struct {
	uint8_t LTK[16];
	uint8_t SK[16];
	uint8_t IV[8];
	uint8_t nonce[13];
	ccm_ctx  ctx;
} btc_ll_enc_ctx_t;
void btc_ll_enc_ctx(uint8_t *SKDm, uint8_t *SKDs, uint8_t *LTK,
		uint8_t *IVm, uint8_t *IVs, btc_ll_enc_ctx_t *ctx);
void btc_ll_encrypt(btc_ll_enc_ctx_t *ctx, const uint32_t counter, const uint8_t isMaster,
		uint8_t LLID, uint8_t *data, uint8_t len, uint8_t *out, uint8_t *mic);
int btc_ll_decrypt(btc_ll_enc_ctx_t *ctx, uint8_t LLID, unsigned char *data,
		unsigned long len, uint8_t *out, uint8_t *mic);
uint32_t btc_crc24(uint32_t crc, uint8_t *buf, int len);
void btc_whitening(uint8_t* data, uint8_t len, uint8_t ch_idx);

#endif /* __BT_ENCRYPT_H__ */

