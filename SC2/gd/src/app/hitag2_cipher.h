#ifndef _HITAG2_CIPHER_H_
#define _HITAG2_CIPHER_H_
#include <stdint.h>
void hitag2_3_set_random(const unsigned char * addr_rand);
void hitag2_oneway2(unsigned char * addr, unsigned char bits);
uint8_t hitag2_1_set_id(const uint8_t *p_id, uint8_t bit_len);
uint8_t hitag2_2_set_sk(const uint8_t *p_sk, uint8_t bit_len);
void hitag3_3_set_random(const unsigned char * addr_rand);
#endif

