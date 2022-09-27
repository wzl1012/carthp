 #include "headfile.h" 

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

static uint64_t hitag3_init (const uint64_t key1, const uint64_t key2,const uint32_t serial, const uint64_t IV)
{
	uint32_t					i;
	uint64_t					x = ((key1 & 0xFFFFFFFF) << 32) + serial;
	
	for (i = 0; i < 64; i++)
	{
		x >>= 1;
		x += (uint64_t) (f20 (x) ^ (((IV >> i) ^ (key2>>i /*>> (i+16)*/)) & 1)) << 63;
	}
	return x;
}

static uint64_t hitag3_round (uint64_t *state)
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
static uint32_t hitag3_byte (uint64_t * x)
{
	uint32_t					i, c;
	
	for (i = 0, c = 0; i < 8; i++) c += (uint32_t) hitag3_round (x) << (i^7);
	return c;
}
uint8_t hitag3_cipher_authenticate(uint64_t* cs, uint8_t* authenticator_is)
{
	         // uint64_t					x = *cs;
            authenticator_is[0] = (uint8_t)~(hitag3_byte(cs));
            authenticator_is[1] = (uint8_t)~(hitag3_byte(cs));
            authenticator_is[2] = (uint8_t)~(hitag3_byte(cs));
            authenticator_is[3] = (uint8_t)~(hitag3_byte(cs));
            return 0;
}
uint8_t hitag3_cipher_transcrypt(uint64_t* cs, uint8_t* data, uint8_t bytes, uint8_t bits)
{
    uint32_t i;
    for (i = 0; i < bytes; i++) data[i] ^= (uint8_t)hitag3_byte(cs);
    for (i = 0; i < bits; i++) data[bytes] ^= (uint8_t)(hitag3_round(cs) << (7 - i));
    return 0;
}
void hitag3_cipher_reset_7953(pcf7953_tag_t *p_tag) {
   uint64_t key1 = ((uint64_t)p_tag->secret_key[8]) |
                   ((uint64_t)p_tag->secret_key[9] << 8) |
                   ((uint64_t)p_tag->secret_key[10] << 16) |
                   ((uint64_t)p_tag->secret_key[11] << 24);
                  // ((uint64_t)p_tag->secret_key[2]<< 32) |
                  // ((uint64_t)p_tag->secret_key[3] << 40);
	uint64_t key2 = ((uint64_t)p_tag->secret_key[0]) |
                   ((uint64_t)p_tag->secret_key[1] << 8) |
                   ((uint64_t)p_tag->secret_key[2] << 16) |
                   ((uint64_t)p_tag->secret_key[3] << 24) |
                   ((uint64_t)p_tag->secret_key[4]<< 32) |
                   ((uint64_t)p_tag->secret_key[5] << 40)|
	                 ((uint64_t)p_tag->secret_key[6] << 48)|
	                 ((uint64_t)p_tag->secret_key[7] << 56);
  uint32_t uid = ((uint32_t)p_tag->a_page[0][0]) |
                   ((uint32_t)p_tag->a_page[0][1]<< 8) |
                   ((uint32_t)p_tag->a_page[0][2] << 16) |
                   ((uint32_t)p_tag->a_page[0][3] << 24);
   uint64_t iv_ = (((uint64_t)(p_tag->random[0]))) |
                   (((uint64_t)(p_tag->random[1])) << 8) |
                   (((uint64_t)(p_tag->random[2])) << 16) |
                   (((uint64_t)(p_tag->random[3])) << 24)|
	                 (((uint64_t)(p_tag->random[4])) << 32)|
	                 (((uint64_t)(p_tag->random[5])) << 40);
   state_7953 = hitag3_init(REV32(key1), REV64(key2),REV32(uid), REV64(iv_));
}
