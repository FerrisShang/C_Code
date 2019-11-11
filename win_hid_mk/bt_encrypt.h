#ifndef __BT_ENCRYPT_H__
#define __BT_ENCRYPT_H__

#include "stdint.h"
void btc_e(uint8_t *key, uint8_t *in_out);
void btc_c1(uint8_t *k, uint8_t *r, uint8_t *preq, uint8_t *pres,
			uint8_t iat, uint8_t rat, uint8_t *ia, uint8_t *ra, uint8_t *out);

#define btc_confirm_value(tk, rand, req_cmd, rep_cmd, init_dev_addr_type, init_dev_addr, rsp_dev_addr_type, rsp_dev_addr, out) \
		btc_c1(tk, rand, req_cmd, rep_cmd, init_dev_addr_type, rsp_dev_addr_type, init_dev_addr, rsp_dev_addr, out);

#endif /* __BT_ENCRYPT_H__ */

