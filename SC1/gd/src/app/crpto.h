#ifndef _CRPTO_H_
#define _CRPTO_H_
#include "headfile.h"
extern uint64_t state;
uint8_t REV8(uint8_t x);
uint32_t REV32(uint32_t x);
uint64_t REV64(uint64_t x);
void hitag2_cipher_reset(pcf7936_tag_t *p_tag);
void hitag2_cipher_reset_7953(pcf7953_tag_t *p_tag);
uint32_t I4(uint64_t x, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
uint8_t hitag2_cipher_authenticate(uint64_t* cs, uint8_t* authenticator_is);
uint8_t hitag2_cipher_transcrypt(uint64_t* cs, uint8_t* data, uint8_t bytes, uint8_t bits);


#endif



