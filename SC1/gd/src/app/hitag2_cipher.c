#include "headfile.h"

/****************************************************************************
* Main parameters for HITAG2 cryptographic algorithm. 
****************************************************************************/
static unsigned char ident[4];				/* Transponder identifier */
static unsigned char secret_key[12];			/* immobilizer or remote secret key */

#define A		(1 + 16)
#define B		(2 + 4 + 8)
static unsigned char F0_table[16] = {A, 0, 0, A, A, A, A, 0, 0, 0, A, A, 0, A, 0, 0};
static unsigned char F1_table[16] = {B, 0, 0, 0, B, B, B, 0, 0, B, B, 0, 0, B, B, 0};
static unsigned char F2_table[32] = {1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0,
										1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0};
#undef A
#undef B

static unsigned char exor_table[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
static unsigned char t[2];
static unsigned char s[6];

static unsigned char GETBIT(unsigned char data,unsigned char bit)
{
  return((data & (1<<bit)) >> bit);  
}

static unsigned char TEST(unsigned char data)
{
	if(data == 0x00)
		return 0x00;
	else
		return 0x01;
}

static unsigned char function_bit(void)	      /*48bits数据查表运算后得到1bit数据返回 */
{
	unsigned char F01_index;
	unsigned char F2_index;

	F01_index  = GETBIT(t[0], 1);
	F01_index <<= 1;
	F01_index |= GETBIT(t[0], 2);
	F01_index <<= 1;
	F01_index |= GETBIT(t[0], 4);
	F01_index <<= 1;
	F01_index |= GETBIT(t[0], 5);
	F2_index  = F0_table[F01_index] & (unsigned char)0x01;

	F01_index  = GETBIT(t[1], 0);
	F01_index <<= 1;
	F01_index |= GETBIT(t[1], 1);
	F01_index <<= 1;
	F01_index |= GETBIT(t[1], 3);
	F01_index <<= 1;
	F01_index |= GETBIT(t[1], 7);
	F2_index |= F1_table[F01_index] & (unsigned char)0x02;

	F01_index  = GETBIT(s[1], 5);
	F01_index <<= 1;
	F01_index |= GETBIT(s[0], 0);
	F01_index <<= 1;
	F01_index |= GETBIT(s[0], 2);
	F01_index <<= 1;
	F01_index |= GETBIT(s[0], 6);
	F2_index |= F1_table[F01_index] & (unsigned char)0x04;

	F01_index  = GETBIT(s[2], 6);
	F01_index <<= 1;
	F01_index |= GETBIT(s[1], 0);
	F01_index <<= 1;
	F01_index |= GETBIT(s[1], 2);
	F01_index <<= 1;
	F01_index |= GETBIT(s[1], 3);
	F2_index |= F1_table[F01_index] & (unsigned char)0x08;

	F01_index  = GETBIT(s[3], 1);
	F01_index <<= 1;
	F01_index |= GETBIT(s[3], 3);
	F01_index <<= 1;
	F01_index |= GETBIT(s[3], 4);
	F01_index <<= 1;
	F01_index |= GETBIT(s[2], 5);
	F2_index |= F0_table[F01_index] & (unsigned char)0x10;
	return F2_table[F2_index];
}

static void shift_reg( unsigned char shift_bit )	/*48bits参数移位*/
{
	t[0] <<= 1;
	t[0] |= GETBIT(t[1], 7);

	t[1] <<= 1;
	t[1] |= GETBIT(s[0], 7);

	s[0] <<= 1;
	s[0] |= GETBIT(s[1], 7);

	s[1] <<= 1;
	s[1] |= GETBIT(s[2], 7);

	s[2] <<= 1;
	s[2] |= GETBIT(s[3], 7);

	s[3] <<= 1;
	s[3] |= shift_bit;
}
static void shift_reg64( unsigned char shift_bit )	/*48bits参数移位*/
{
	t[0] <<= 1;
	t[0] |= GETBIT(t[1], 7);

	t[1] <<= 1;
	t[1] |= GETBIT(s[0], 7);

	s[0] <<= 1;
	s[0] |= GETBIT(s[1], 7);

	s[1] <<= 1;
	s[1] |= GETBIT(s[2], 7);

	s[2] <<= 1;
	s[2] |= GETBIT(s[3], 7);
	
	s[3] <<= 1;
	s[3] |= GETBIT(s[4], 7);
	
	s[4] <<= 1;
	s[4] |= GETBIT(s[5], 7);
	
	s[5] <<= 1;
	s[5] |= shift_bit;
}

static unsigned char feed_back(void)	   /*48bits数据抽取16bit数据进行异或后得到1bit数据*/
{
	unsigned char sum;

	sum = (t[0] & (unsigned char)0xB3) ^ (t[1] & (unsigned char)0x80) ^
			(s[0] & (unsigned char)0x83) ^ (s[1] & (unsigned char)0x22) ^ (s[3] & (unsigned char)0x73);

	return exor_table[sum%16] ^ exor_table[sum/16];
}

/****************************************************************************
* 
* Description: 
*   Performs the initialization phase of the HITAG2/HITAG2+ cryptographic
*   algorithm. This function can be used to implement the cryptographic
*   protocol in transponder mode (HITAG2/HITAG2+) and remote mode (HITAG2+)
*   Depending on the mode of operation, the initialization is done with
*   different input parameters. 
* 
*   a) Transponder Mode:
*   The feedback registers are loaded with identifier and immobilizer
*   secret key, and oneway function 1 is executed 32 times thereafter. 
*   The global variables ident and secretkey are used for identifier,
*   immobilizer secret key, the random number is given as an input
*   to the function. 
*
*   b) Remote Mode:
*   The feedback registers are loaded with identifier and remote
*   secret key, and oneway function 1 is executed 32 times thereafter.
*   The global variables HT2ident and HT2secretkey are used for identifier,
*   remote secret key. The sequence increment (28bit) and the command ID
*   (4bit) are given together as 32bit input data block to the function.
* 
*   The function outputs the initialized global shift registers t,s.
* 
* Parameters: 
*   addr_rand: Pointer to a memory area of 4 bytes that contains the
*              random number (transponder mode)
*              or sequence increment + command ID (remote mode)
*              to be used for the initialization. 
* 
* Return: none 
* 
****************************************************************************/
void hitag2_3_set_random(const unsigned char * addr_rand)	/*hitag2 initial*/
{
	unsigned char bit_mask;
	unsigned char byte_cnt;
	unsigned char fb;

	/* Initialise oneway function 1 with identifier and parts of secret key */
	t[0] = ident[0];
	t[1] = ident[1];
	s[0] = ident[2];
	s[1] = ident[3];
	s[2] = secret_key[4];
	s[3] = secret_key[5];
  
	byte_cnt = 0;
	bit_mask = 0x80;		/* Setup bit mask: MSB of first byte */
	do {
		fb = function_bit() ^ TEST((secret_key[byte_cnt] ^ addr_rand[byte_cnt]) & bit_mask);
		shift_reg(fb);
		bit_mask >>= 1;
		if (bit_mask == 0) {
			bit_mask = 0x80;
			byte_cnt++;
		}
	} while(byte_cnt < 4);
				for(uint8_t i=0;i<4;i++)
			 debug_printf("\r\ns[%d]=0x%02x\r\n",i,s[i]);
}
void hitag3_3_set_random(const unsigned char * addr_rand)	/*hitag3 initial*/
{
	unsigned char bit_mask;
	unsigned char byte_cnt;
	unsigned char fb;

	/* Initialise oneway function 1 with identifier and parts of secret key */
	t[0] = ident[0];
	t[1] = ident[1];
	s[0] = ident[2];
	s[1] = ident[3];
	s[2] = secret_key[8];
	s[3] = secret_key[9];
  s[4] =secret_key[10];
	s[5] =secret_key[11];
	byte_cnt = 0;
	bit_mask = 0x80;		/* Setup bit mask: MSB of first byte */
	do {
		fb = function_bit() ^ TEST((secret_key[byte_cnt] ^ addr_rand[byte_cnt]) & bit_mask);
		shift_reg64(fb);
		bit_mask >>= 1;
		if (bit_mask == 0) {
			bit_mask = 0x80;
			byte_cnt++;
		}
	} while(byte_cnt < 8);
	      for(uint8_t i=0;i<2;i++)
	      debug_printf("\r\nt[%d]=0x%02x\r\n",i,t[i]);
				for(uint8_t i=0;i<4;i++)
			 debug_printf("\r\ns[%d]=0x%02x\r\n",i,s[i]);
}
/****************************************************************************
* 
* Description: 
*   Performs the encryption respective decryption of a given data block
*   by repeatedly executing oneway function 2 and Exclusive-Oring with the
*   generated cipher bits. The computation is repeated for the number
*   of bits as specified by "length". The global shift register contents 
*   after the initialization by hitag2_3_set_random(), or the current contents after
*   the last call of hitag2_oneway2() is used as start condition.
*  
* Parameters:
*   addr:   Pointer to a memory area that contains the data to be
*           encrypted or decrypted. 
*   length: Number of bits to encrypt / decrypt.
*   
* Return: none  
* 
****************************************************************************/
void hitag2_oneway2(unsigned char * addr, unsigned char bits)
{
	unsigned char bit_mask;
	unsigned char bitval;

	bit_mask = 0x80;
	do {
		bitval = (unsigned char)((function_bit() ^ TEST(*addr & bit_mask)) * bit_mask);
		*addr = (*addr & (unsigned char)~bit_mask) | bitval;

		shift_reg(feed_back());

		bit_mask >>= 1;
		if (bit_mask == 0) {
			bit_mask = 0x80;
			addr++;
		}

		bits--;
	} while(bits);
}

/********************************************************
* 32bit , MSBit first, MSBit-->LSBit
********************************************************/
uint8_t hitag2_1_set_id(const uint8_t *p_id, uint8_t bit_len)
{
	if (bit_len != 32)
		return FALSE;
	
	ident[0] = p_id[0];		//MSB
	ident[1] = p_id[1];
	ident[2] = p_id[2];
	ident[3] = p_id[3];		//LSB
	
	return TRUE;
}

/********************************************************
* 48bit , MSBit first, MSBit-->LSBit
********************************************************/
uint8_t hitag2_2_set_sk(const uint8_t *p_sk,uint8_t bit_len)
{
	if (bit_len != 48)
		return FALSE;
	//if(bit_len==48){
	secret_key[0]=p_sk[0];		//0x4D; 		//LMSB
	secret_key[1]=p_sk[1];		//0x49;
	secret_key[2]=p_sk[2];		//0x4B;
	secret_key[3]=p_sk[3];		//0x52; 		//LLSB 
	secret_key[4]=p_sk[4];		//0x4F; 		//HMSB
	secret_key[5]=p_sk[5];		//0x4E; 		//HLSB
/*	}else if(bit_len==96){
	secret_key[0]=p_sk[0];		//0x4D; 		//LMSB
	secret_key[1]=p_sk[1];		//0x49;
	secret_key[2]=p_sk[2];		//0x4B;
	secret_key[3]=p_sk[3];		//0x52; 		//LLSB 
	secret_key[4]=p_sk[4];		//0x4F; 		//HMSB
	secret_key[5]=p_sk[5];
		secret_key[6]=p_sk[6];
		secret_key[7]=p_sk[7];
		secret_key[8]=p_sk[8];
		secret_key[9]=p_sk[9];
		secret_key[10]=p_sk[10];
		secret_key[11]=p_sk[11];
	}*/
	return TRUE;
}

#if 0
	id = 1acb0a73;

	key = 524B494D4E4F;

	iv = 6e6f4671;

	signature = 51b8ae69;

	uint8_t a_id[4], a_sk[6];
	uint8_t a_random[4], a_encry[6];
	
	a_id[0] = 0xDF;
	a_id[1] = 0x03;
	a_id[2] = 0x95;
	a_id[3] = 0x14;
	hitag2_1_set_id(a_id, 32);
	
	a_sk[0]=0x19;
	a_sk[1]=0x73;
	a_sk[2]=0x07;
	a_sk[3]=0x23;
	a_sk[4]=0x0A;
	a_sk[5]=0x0E;
	hitag2_2_set_sk(a_sk, 48);
	
	a_random[0] = 0x62;
	a_random[1] = 0x3F;
	a_random[2] = 0xAB;
	a_random[3] = 0x30;
	hitag2_3_set_random(a_random);
	
	a_encry[0] = 0x07;
	a_encry[1] = 0xE4;
	a_encry[2] = 0xDE;
	a_encry[3] = 0x2A;
	hitag2_oneway2(a_encry, 32);
	
	a_encry[0] = 0x25;
	a_encry[1] = 0xA3;
	a_encry[2] = 0x73;
	a_encry[3] = 0x3D;
	hitag2_oneway2(a_encry, 32);
	
	a_encry[0] = 0xEC;
	a_encry[1] = 0x40;
	hitag2_oneway2(a_encry, 10);
	
	a_encry[0] = 0x03;
	a_encry[1] = 0x75;
	a_encry[2] = 0x36;
	a_encry[3] = 0x11;
	hitag2_oneway2(a_encry, 32);
	
	
#endif

