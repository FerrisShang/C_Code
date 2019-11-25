#include "stdio.h"
#include "string.h"
#include "bt_encrypt.h"

#include "alg/cmac.h"
void test(void)
{
	uint8_t out[32];
#ifndef DUMP
#define DUMP(d,l) do{int i;for(i=0;i<l;i++)printf("%02X ", d[i]); puts("");}while(0)
#endif
	/******************************************************
	  D.1 AES-CMAC RFC4493 TEST VECTORS
	  The following test vectors are referenced from RFC4493.
	  K 2b7e1516 28aed2a6 abf71588 09cf4f3c
	  Subkey Generation
	  AES_128(key,0) 7df76b0c 1ab899b3 3e42f047 b91b546f
	  K1 fbeed618 35713366 7c85e08f 7236a8de
	  K2 f7ddac30 6ae266cc f90bc11e e46d513b
	 ******************************************************/
	{
		uint8_t k[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		memset(out, 0, 16); btc_e(k, out);
		DUMP(out, 16);
	}

	/******************************************************
	  D.1.1 Example 1: Len = 0
	  M <empty string>
	  AES_CMAC bb1d6929 e9593728 7fa37d12 9b756746
	 ******************************************************/
	{
		uint8_t k[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		btc_aes_cmac(k, NULL, 0, out);
		DUMP(out, 16);
	}

	/******************************************************
	  D.1.2 Example 2: Len = 16
	  M 6bc1bee2 2e409f96 e93d7e11 7393172a
	  AES_CMAC 070a16b4 6b4d4144 f79bdd9d d04a287c
	 ******************************************************/
	{
		uint8_t k[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		uint8_t d[] = {
			0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,
		};
		btc_aes_cmac(k, d, 16, out);
		DUMP(out, 16);
	}

	/******************************************************
	  D.1.3 Example 3: Len = 40
	  M0 6bc1bee2 2e409f96 e93d7e11 7393172a
	  M1 ae2d8a57 1e03ac9c 9eb76fac 45af8e51
	  M2 30c81c46 a35ce411
	  AES_CMAC dfa66747 de9ae630 30ca3261 1497c827
	 ******************************************************/
	{
		uint8_t k[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		uint8_t d[] = {
			0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,
			0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51,
			0x30,0xc8,0x1c,0x46,0xa3,0x5c,0xe4,0x11,
		};
		btc_aes_cmac(k, d, 40, out);
		DUMP(out, 16);
	}

	/******************************************************
	  D.1.4 Example 4: Len = 64
	  M0 6bc1bee2 2e409f96 e93d7e11 7393172a
	  M1 ae2d8a57 1e03ac9c 9eb76fac 45af8e51
	  M2 30c81c46 a35ce411 e5fbc119 1a0a52ef
	  M3 f69f2445 df4f9b17 ad2b417b e66c3710
	  AES_CMAC 51f0bebf 7e3b9d92 fc497417 79363cfe
	 ******************************************************/
	{
		uint8_t k[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
		uint8_t d[] = {
			0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,
			0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51,
			0x30,0xc8,0x1c,0x46,0xa3,0x5c,0xe4,0x11,0xe5,0xfb,0xc1,0x19,0x1a,0x0a,0x52,0xef,
			0xf6,0x9f,0x24,0x45,0xdf,0x4f,0x9b,0x17,0xad,0x2b,0x41,0x7b,0xe6,0x6c,0x37,0x10,
		};
		btc_aes_cmac(k, d, 64, out);
		DUMP(out, 16);
	}

	/******************************************************
	  D.2 f4 LE SC CONFIRM VALUE GENERATION FUNCTION
	  U 20b003d2 f297be2c 5e2c83a7 e9f9a5b9
	  eff49111 acf4fddb cc030148 0e359de6
	  V 55188b3d 32f6bb9a 900afcfb eed4e72a
	  59cb9ac2 f19d7cfb 6b4fdd49 f47fc5fd
	  X d5cb8454 d177733e ffffb2ec 712baeab
	  Z 0x00
	  M0 20b003d2 f297be2c 5e2c83a7 e9f9a5b9
	  M1 eff49111 acf4fddb cc030148 0e359de6
	  M2 55188b3d 32f6bb9a 900afcfb eed4e72a
	  M3 59cb9ac2 f19d7cfb 6b4fdd49 f47fc5fd
	  00
	  AES_CMAC f2c916f1 07a9bd1c f1eda1be a974872d
	 ******************************************************/
	{
		uint8_t u[] = {
			0x20,0xb0,0x03,0xd2,0xf2,0x97,0xbe,0x2c,0x5e,0x2c,0x83,0xa7,0xe9,0xf9,0xa5,0xb9,
			0xef,0xf4,0x91,0x11,0xac,0xf4,0xfd,0xdb,0xcc,0x03,0x01,0x48,0x0e,0x35,0x9d,0xe6,
		};
		uint8_t v[] = {
			0x55,0x18,0x8b,0x3d,0x32,0xf6,0xbb,0x9a,0x90,0x0a,0xfc,0xfb,0xee,0xd4,0xe7,0x2a,
			0x59,0xcb,0x9a,0xc2,0xf1,0x9d,0x7c,0xfb,0x6b,0x4f,0xdd,0x49,0xf4,0x7f,0xc5,0xfd,
		};
		uint8_t x[] = {
			0xd5,0xcb,0x84,0x54,0xd1,0x77,0x73,0x3e,0xff,0xff,0xb2,0xec,0x71,0x2b,0xae,0xab,
		};
		uint8_t z = 0;
		btc_f4(u, v, x, z, out);
		DUMP(out, 16);
	}

	/******************************************************
	  D.3 f5 LE SC KEY GENERATION FUNCTION
	  DHKey(W) ec0234a3 57c8ad05 341010a6 0a397d9b
	  99796b13 b4f866f1 868d34f3 73bfa698
	  T 3c128f20 de883288 97624bdb 8dac6989
	  keyID 62746c65
	  N1 d5cb8454 d177733e ffffb2ec 712baeab
	  N2 a6e8e7cc 25a75f6e 216583f7 ff3dc4cf
	  A1 00561237 37bfce
	  A2 00a71370 2dcfc1
	  Length 0100
	  (LTK)
	  M0 0162746c 65d5cb84 54d17773 3effffb2
	  M1 ec712bae aba6e8e7 cc25a75f 6e216583
	  M2 f7ff3dc4 cf005612 3737bfce 00a71370
	  M3 2dcfc101 00
	  AES_CMAC 69867911 69d7cd23 980522b5 94750a38
	  (MacKey)
	  M0 0062746c 65d5cb84 54d17773 3effffb2
	  M1 ec712bae aba6e8e7 cc25a75f 6e216583
	  M2 f7ff3dc4 cf005612 3737bfce 00a71370
	  M3 2dcfc101 00
	  AES_CMAC 2965f176 a1084a02 fd3f6a20 ce636e20
	 ******************************************************/
	{
		uint8_t w[] = {
			0xec,0x02,0x34,0xa3,0x57,0xc8,0xad,0x05,0x34,0x10,0x10,0xa6,0x0a,0x39,0x7d,0x9b,
			0x99,0x79,0x6b,0x13,0xb4,0xf8,0x66,0xf1,0x86,0x8d,0x34,0xf3,0x73,0xbf,0xa6,0x98,
		};
		uint8_t n1[] = {
			0xd5,0xcb,0x84,0x54,0xd1,0x77,0x73,0x3e,0xff,0xff,0xb2,0xec,0x71,0x2b,0xae,0xab,
		};
		uint8_t n2[] = {
			0xa6,0xe8,0xe7,0xcc,0x25,0xa7,0x5f,0x6e,0x21,0x65,0x83,0xf7,0xff,0x3d,0xc4,0xcf,
		};
		uint8_t a1[] = { 0x00,0x56,0x12,0x37,0x37,0xbf,0xce, };
		uint8_t a2[] = { 0x00,0xa7,0x13,0x70,0x2d,0xcf,0xc1, };
		btc_f5(w, n1, n2, a1, a2, out);
		DUMP(((uint8_t*)&out[16]), 16);
		DUMP(((uint8_t*)&out[0]), 16);
	}


	/******************************************************
	  D.4 f6 LE SC CHECK VALUE GENERATION FUNCTION
	  N1 d5cb8454 d177733e ffffb2ec 712baeab
	  N2 a6e8e7cc 25a75f6e 216583f7 ff3dc4cf
	  MacKey 2965f176 a1084a02 fd3f6a20 ce636e20
	  R 12a3343b b453bb54 08da42d2 0c2d0fc8
	  IOcap 010102
	  A1 00561237 37bfce
	  A2 00a71370 2dcfc1
	  M0 d5cb8454 d177733e ffffb2ec 712baeab
	  M1 a6e8e7cc 25a75f6e 216583f7 ff3dc4cf
	  M2 12a3343b b453bb54 08da42d2 0c2d0fc8
	  M3 01010200 56123737 bfce00a7 13702dcf
	  M4 c1
	  AES_CMAC e3c47398 9cd0e8c5 d26c0b09 da958f61
	 ******************************************************/
	{
		uint8_t n1[] = {0xd5,0xcb,0x84,0x54,0xd1,0x77,0x73,0x3e,0xff,0xff,0xb2,0xec,0x71,0x2b,0xae,0xab,};
		uint8_t n2[] = {0xa6,0xe8,0xe7,0xcc,0x25,0xa7,0x5f,0x6e,0x21,0x65,0x83,0xf7,0xff,0x3d,0xc4,0xcf,};
		uint8_t MacKey[] = {0x29,0x65,0xf1,0x76,0xa1,0x08,0x4a,0x02,0xfd,0x3f,0x6a,0x20,0xce,0x63,0x6e,0x20,};
		uint8_t r[] = {0x12,0xa3,0x34,0x3b,0xb4,0x53,0xbb,0x54,0x08,0xda,0x42,0xd2,0x0c,0x2d,0x0f,0xc8,};
		uint8_t IOcap[] = {0x01,0x01,0x02,};
		uint8_t a1[] = { 0x00,0x56,0x12,0x37,0x37,0xbf,0xce, };
		uint8_t a2[] = { 0x00,0xa7,0x13,0x70,0x2d,0xcf,0xc1, };
		btc_f6(MacKey, n1, n2, r, IOcap, a1, a2, out);
		DUMP(out, 16);
	}


	/******************************************************
	  D.5 g2 LE SC NUMERIC COMPARISON GENERATION
	  FUNCTION
	  U 20b003d2 f297be2c 5e2c83a7 e9f9a5b9
	  eff49111 acf4fddb cc030148 0e359de6
	  V 55188b3d 32f6bb9a 900afcfb eed4e72a
	  59cb9ac2 f19d7cfb 6b4fdd49 f47fc5fd
	  X d5cb8454 d177733e ffffb2ec 712baeab
	  Y a6e8e7cc 25a75f6e 216583f7 ff3dc4cf
	  M0 20b003d2 f297be2c 5e2c83a7 e9f9a5b9
	  M1 eff49111 acf4fddb cc030148 0e359de6
	  M2 55188b3d 32f6bb9a 900afcfb eed4e72a
	  M3 59cb9ac2 f19d7cfb 6b4fdd49 f47fc5fd
	  M4 a6e8e7cc 25a75f6e 216583f7 ff3dc4cf
	  AES_CMAC 1536d18d e3d20df9 9b7044c1 2f9ed5ba
	  g2 2f9ed5ba
	 ******************************************************/
	{
		uint8_t u[] = {
			0x20,0xb0,0x03,0xd2,0xf2,0x97,0xbe,0x2c,0x5e,0x2c,0x83,0xa7,0xe9,0xf9,0xa5,0xb9,
			0xef,0xf4,0x91,0x11,0xac,0xf4,0xfd,0xdb,0xcc,0x03,0x01,0x48,0x0e,0x35,0x9d,0xe6,
		};
		uint8_t v[] = {
			0x55,0x18,0x8b,0x3d,0x32,0xf6,0xbb,0x9a,0x90,0x0a,0xfc,0xfb,0xee,0xd4,0xe7,0x2a,
			0x59,0xcb,0x9a,0xc2,0xf1,0x9d,0x7c,0xfb,0x6b,0x4f,0xdd,0x49,0xf4,0x7f,0xc5,0xfd,
		};
		uint8_t x[] = {0xd5,0xcb,0x84,0x54,0xd1,0x77,0x73,0x3e,0xff,0xff,0xb2,0xec,0x71,0x2b,0xae,0xab,};
		uint8_t y[] = {0xa6,0xe8,0xe7,0xcc,0x25,0xa7,0x5f,0x6e,0x21,0x65,0x83,0xf7,0xff,0x3d,0xc4,0xcf,};
		printf("0x%08x\n", btc_g2(u, v, x, y));
	}

	/******************************************************
	  D.6 h6 LE SC LINK KEY CONVERSION FUNCTION
	  Key ec0234a3 57c8ad05 341010a6 0a397d9b
	  keyID 6c656272
	  M 6c656272
	  AES_CMAC 2d9ae102 e76dc91c e8d3a9e2 80b16399
	 ******************************************************/
	{
		uint8_t Key[] = {0xec,0x02,0x34,0xa3,0x57,0xc8,0xad,0x05,0x34,0x10,0x10,0xa6,0x0a,0x39,0x7d,0x9b,};
		uint8_t keyID[] = { 0x6c,0x65,0x62,0x72, };
		btc_h6(Key, keyID, out);
		DUMP(out, 16);
	}

	/******************************************************
	  D.7 ah RANDOM ADDRESS HASH FUNCTIONS
	  IRK ec0234a3 57c8ad05 341010a6 0a397d9b
	  prand 00000000 00000000 00000000 00708194
	  M 00000000 00000000 00000000 00708194
	  AES_128 159d5fb7 2ebe2311 a48c1bdc c40dfbaa
	  ah 0dfbaa
	 ******************************************************/
	{
		uint8_t IRK[] = {0xec,0x02,0x34,0xa3,0x57,0xc8,0xad,0x05,0x34,0x10,0x10,0xa6,0x0a,0x39,0x7d,0x9b,};
		uint8_t prand[] = {0x70,0x81,0x94,};
		btc_ah(IRK, prand, out);
		DUMP(out, 3);
	}

	/******************************************************
	  D.8 h7 LE SC LINK KEY CONVERSION FUNCTION
	  Key ec0234a3 57c8ad05 341010a6 0a397d9b
	  SALT 00000000 00000000 00000000 746D7031
	  AES_CMAC fb173597 c6a3c0ec d2998c2a 75a57011
	 ******************************************************/
	{
		uint8_t Key[] = {0xec,0x02,0x34,0xa3,0x57,0xc8,0xad,0x05,0x34,0x10,0x10,0xa6,0x0a,0x39,0x7d,0x9b,};
		uint8_t SALT[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x74,0x6D,0x70,0x31,};
		btc_h7(SALT, Key, out);
		DUMP(out, 16);
	}

	{
		uint8_t out[32];
		uint8_t tk[16] = {0};
		uint8_t rand[] = {0xF5,0xCE,0xFC,0xDF,0xA4,0xDB,0x2B,0x5B,0x0F,0xC0,0x3A,0x94,0x47,0x9A,0xA2,0x49};
		uint8_t req[] = {0x01,0x04,0x00,0x01,0x10,0x03,0x03};
		uint8_t resp[] = {0x02,0x03,0x00,0x01,0x10,0x01,0x01};
		uint8_t iat = 1;
		uint8_t rat = 0;
		uint8_t ia[] = {0x8F,0xE1,0x2D,0x7D,0xA8,0x46};
		uint8_t ra[] = {0x78,0x28,0x00,0x86,0x19,0x00};
		btc_confirm_value(tk, rand, req, resp, iat, ia, rat, ra, out);
		/* 09 2A B4 A8 C1 B8 8F 3A 6E 97 9A 87 55 C3 18 0B */
		DUMP(out, 16);
	}

	{
		memcpy(out, "66666666666666666", 17);
		btc_e((uint8_t*)"1234567890123456", out);
		DUMP(out, 16);
	}

}

int main()
{
	test();
}
