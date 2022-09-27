 #include "headfile.h" 
 
//static unsigned char ident[4];				/* Transponder identifier */
//static unsigned char secret_key[12];			/* immobilizer or remote secret key */
uint64_t state=0;
uint32_t I4(uint64_t x, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
  {
            uint32_t v =(uint32_t)((((x) >> (a)) & 1) + (((x) >> (b)) & 1) * 2 + (((x) >> (c)) & 1) * 4 + (((x) >> (d)) & 1) * 8);

            return v;
  }
uint8_t REV8(uint8_t x)
    {
           uint8_t y= (uint8_t)((((x) >> 7) & 1) + ((((x) >> 6) & 1) << 1) + ((((x) >> 5) & 1) << 2) + ((((x) >> 4) & 1) << 3) + ((((x) >> 3) & 1) << 4) + ((((x) >> 2) & 1) << 5) + ((((x) >> 1) & 1) << 6) + (((x) & 1) << 7));
            return y;
    }


uint16_t REV16(uint16_t x)
  {
          uint16_t y = REV8 (x)+(REV8 (x>> 8)<< 8);
            return y;
  }

uint32_t REV32(uint32_t x)
 {
      uint32_t y =REV16(x)+(REV16(x>>16)<<16);
       return y;
 }

uint64_t REV64(uint64_t x)
 {
	   uint32_t low = REV32((uint32_t)x);
     uint32_t high = REV32((uint32_t)((x) >> 32));
     uint64_t shift = (uint64_t)(((uint64_t)high) << 32);
     uint64_t y = (uint64_t)(((uint64_t)low) + shift);
   // uint64_t y = REV32(x)+(REV32(x>>32)<<32);
      return y;
 }	
static const uint32_t ht2_f4a = 0x2C79;		// 0010 1100 0111 1001
static const uint32_t ht2_f4b = 0x6671;		// 0110 0110 0111 0001
static const uint32_t ht2_f5c = 0x7907287B;	// 0111 1001 0000 0111 0010 1000 0111 1011

static uint32_t f20 (const uint64_t x)
{
	uint32_t					i5;
	
	i5 = ((ht2_f4a >> I4 (x, 1, 2, 4, 5)) & 1)* 1
	   + ((ht2_f4b >> I4  (x, 7,11,13,14)) & 1)* 2
	   + ((ht2_f4b >> I4  (x,16,20,22,25)) & 1)* 4
	   + ((ht2_f4b >> I4  (x,27,28,30,32)) & 1)* 8
	   + ((ht2_f4a >> I4  (x,33,42,43,45)) & 1)*16;
	
	return (ht2_f5c >> i5) & 1;
}

static uint64_t hitag2_init (const uint64_t key, const uint32_t serial, const uint32_t IV)
{
	uint32_t					i;
	uint64_t					x = ((key & 0xFFFF) << 32) + serial;
	
	for (i = 0; i < 32; i++)
	{
		x >>= 1;
		x += (uint64_t) (f20 (x) ^ (((IV >> i) ^ (key >> (i+16))) & 1)) << 47;
	}
	return x;
}

static uint64_t hitag2_round (uint64_t *state)
{
	uint64_t					x = *state;
	
	x = (x >>  1) +
	 ((((x >>  0) ^ (x >>  2) ^ (x >>  3) ^ (x >>  6)
	  ^ (x >>  7) ^ (x >>  8) ^ (x >> 16) ^ (x >> 22)
	  ^ (x >> 23) ^ (x >> 26) ^ (x >> 30) ^ (x >> 41)
	  ^ (x >> 42) ^ (x >> 43) ^ (x >> 46) ^ (x >> 47)) & 1) << 47);
	
	*state = x;
	return f20 (x);
}
static uint32_t hitag2_byte (uint64_t * x)
{
	uint32_t					i, c;
	
	for (i = 0, c = 0; i < 8; i++) c += (uint32_t) hitag2_round (x) << (i^7);
	return c;
}
uint8_t hitag2_cipher_authenticate(uint64_t* cs, uint8_t* authenticator_is)
{
	         // uint64_t					x = *cs;
            authenticator_is[0] = (uint8_t)~(hitag2_byte(cs));
            authenticator_is[1] = (uint8_t)~(hitag2_byte(cs));
            authenticator_is[2] = (uint8_t)~(hitag2_byte(cs));
            authenticator_is[3] = (uint8_t)~(hitag2_byte(cs));
            return 0;
}
uint8_t hitag2_cipher_transcrypt(uint64_t* cs, uint8_t* data, uint8_t bytes, uint8_t bits)
{
    uint32_t i;
    for (i = 0; i < bytes; i++) data[i] ^= (uint8_t)hitag2_byte(cs);
    for (i = 0; i < bits; i++) data[bytes] ^= (uint8_t)(hitag2_round(cs) << (7 - i));
    return 0;
}
void hitag2_cipher_reset(pcf7936_tag_t *p_tag) {
   uint64_t key = ((uint64_t)p_tag->secret_key[4]) |
                   ((uint64_t)p_tag->secret_key[5] << 8) |
                   ((uint64_t)p_tag->secret_key[0] << 16) |
                   ((uint64_t)p_tag->secret_key[1] << 24) |
                   ((uint64_t)p_tag->secret_key[2]<< 32) |
                   ((uint64_t)p_tag->secret_key[3] << 40);
  uint32_t uid = ((uint32_t)p_tag->a_page[0][0]) |
                   ((uint32_t)p_tag->a_page[0][1]<< 8) |
                   ((uint32_t)p_tag->a_page[0][2] << 16) |
                   ((uint32_t)p_tag->a_page[0][3] << 24);
   uint32_t iv_ = (((uint32_t)(p_tag->random[0]))) |
                   (((uint32_t)(p_tag->random[1])) << 8) |
                   (((uint32_t)(p_tag->random[2])) << 16) |
                   (((uint32_t)(p_tag->random[3])) << 24);
   state = hitag2_init(REV64(key), REV32(uid), REV32(iv_));
}
void hitag2_cipher_reset_7953(pcf7953_tag_t *p_tag) {
   uint64_t key = ((uint64_t)p_tag->secret_key[4]) |
                   ((uint64_t)p_tag->secret_key[5] << 8) |
                   ((uint64_t)p_tag->secret_key[0] << 16) |
                   ((uint64_t)p_tag->secret_key[1] << 24) |
                   ((uint64_t)p_tag->secret_key[2]<< 32) |
                   ((uint64_t)p_tag->secret_key[3] << 40);
  uint32_t uid = ((uint32_t)p_tag->a_page[0][0]) |
                   ((uint32_t)p_tag->a_page[0][1]<< 8) |
                   ((uint32_t)p_tag->a_page[0][2] << 16) |
                   ((uint32_t)p_tag->a_page[0][3] << 24);
   uint32_t iv_ = (((uint32_t)(p_tag->random[0]))) |
                   (((uint32_t)(p_tag->random[1])) << 8) |
                   (((uint32_t)(p_tag->random[2])) << 16) |
                   (((uint32_t)(p_tag->random[3])) << 24);
   state_7953 = hitag2_init(REV64(key), REV32(uid), REV32(iv_));
}
