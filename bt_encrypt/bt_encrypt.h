#ifndef __BT_ENCRYPT_H__
#define __BT_ENCRYPT_H__
#include "stdint.h"

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

#endif /* __BT_ENCRYPT_H__ */

