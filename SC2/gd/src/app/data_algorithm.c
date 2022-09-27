#include "headfile.h"

/********************************************************
* function: compare sequence and one byte
*
* p_to: pointer for compare sequence
* byte: compare byte
* bytes: bytes of sequence
*
* return: if sequence equal byte, return TRUE; else, return FALSE.
********************************************************/
uint8_t equal_byte(const uint8_t *p_to, const uint8_t byte, uint8_t bytes)
{
	while (bytes > 0) {
		if (*p_to != byte)
			return FALSE;

		p_to++;
		bytes--;
	}

	return TRUE;
}

/********************************************************
* function: compare two sequences data
*
* p_to:		pointer for sequence1
* p_from:	pointer for sequence2
* bytes:	compare bytes
*
* return: if two sequences are equal, return TRUE; else, return FALSE.
********************************************************/
uint8_t equal_bytes(const uint8_t *p_to, const uint8_t *p_from, uint8_t bytes)
{
	while (bytes > 0) {
		if (*p_to != *p_from)
			return FALSE;

		p_to++;
		p_from++;
		bytes--;
	}

	return TRUE;
}

/********************************************************
* function: calculate bytes xor result
*
* p_data: pointer for sequence
* bytes: calculate bytes
*
* return: xor data
********************************************************/
uint8_t xor_bytes(const uint8_t *p_data, uint8_t bytes)
{
	uint8_t xor = 0;

	while (bytes) {
		xor ^= *p_data;
		p_data++;
		bytes--;
	}

	return xor;
}

/********************************************************
* avoid overflow, check parameter "bytes" value before call this function
*
* p_to:		pointer for copy to
* p_from:	pointer for copy from
* bytes:	copy bytes
********************************************************/
void copy_bytes(uint8_t *p_to, const uint8_t *p_from, uint8_t bytes)
{
	while (bytes > 0) {
		*p_to++ = *p_from++;
		bytes--;
	}
}

/**************************************************
* max bits is 255
* 0 <= offset_to <= 7
* 0 <= offset_from <= 7
**************************************************/
void copy_msbits(uint8_t *p_to, uint8_t offset_to, const uint8_t *p_from, uint8_t offset_from, uint8_t bits)
{
	while (bits > 0) {
		if (*p_from & (1 << (7-(offset_from%8))))
			*p_to |= (1 << (7-(offset_to%8)));
		else
			*p_to &= ~(1 << (7-(offset_to%8)));
		
		if ((offset_to%8) == 7)
			p_to++;
		if ((offset_from%8) == 7)
			p_from++;

		offset_to++;
		offset_from++;
		bits--;
	}
}

/**************************************************
* max bits is 255
* 0 <= offset_to <= 7
* 0 <= offset_from <= 7
**************************************************/
void copy_lsbits(uint8_t *p_to, uint8_t offset_to, const uint8_t *p_from, uint8_t offset_from, uint8_t bits)
{
	while (bits > 0) {
		if (*p_from & (1 << ((offset_from%8))))
			*p_to |= (1 << (offset_to%8));
		else
			*p_to &= ~(1 << (offset_to%8));
		
		if ((offset_to % 8) == 7)
			p_to++;
		if ((offset_from % 8) == 7)
			p_from++;

		offset_to++;
		offset_from++;
		bits--;
	}
}

/********************************************************
* p_data[0] = p_data[bytes - 1]
* p_data[1] = p_data[bytes - 2]
* ......
* p_data[bytes - 2] = p_data[1]
* p_data[bytes - 1] = p_data[0]
********************************************************/
void reverse_sequence(uint8_t *p_data, const uint8_t bytes)
{
	uint8_t i, temp;

	for (i = 0; i < (bytes / 2); i++) {
		temp = p_data[i];
		p_data[i] = p_data[bytes - 1 - i];
		p_data[bytes - 1- i] = temp;
	}
}

/********************************************************
* bit7 = bit0
* bit6 = bit1
* ......
* bit1 = bit6
* bit0 = bit7
********************************************************/
uint8_t reverse_byte(uint8_t byte)
{
	byte = (byte<<4) | (byte>>4);
	byte = ((byte<<2) & 0xCC) | ((byte>>2) & 0x33);
	byte = ((byte<<1) & 0xAA) | ((byte>>1) & 0x55);
	
	return byte;
}

/********************************************************
* bit15 = bit0
* bit14 = bit1
* ......
* bit1 = bit14
* bit0 = bit15
********************************************************/
uint16_t reverse_word(uint16_t word)
{
	return (reverse_byte((uint8_t)word) << 8) | (reverse_byte((uint8_t)(word>>8)));
}

uint8_t calculate_msbit_parity(const uint8_t *p_data, uint8_t bits)
{
	uint8_t i, byte=0, cnt;

	cnt = 0;
	for (i = 0; i < bits; i++) {
		if ((i & 0x07) == 0)
			byte = *p_data++;
		
		if (byte & 0x80)
			cnt++;
		byte <<= 1;
	}

	return (cnt & 0x01);
}

/********************************************************
* function: array of hexadecimal convert to integer number
* MSByte is Highest of integer
*
* p_hex: pointer for hexadecimal, need to convert
* bytes: convert bytes
*
* return: convert integer number
********************************************************/
uint64_t hexs_to_dec_msbyte(const uint8_t *p_hex, uint8_t bytes)
{
		
	uint64_t dec = 0;
	uint8_t i;
	/* because return is 64bit, so convert bytes <= 8 */
	if (bytes > 8)
		return 0;
	for (i = 0; i < bytes; i++) {
		dec |= ((uint64_t)p_hex[i]) << ((bytes-1-i) << 3);
	}

	return dec;
}

/********************************************************
* function: array of hexadecimal convert to integer number
* LSByte is Highest of integer
*
* p_hex: pointer for hexadecimal, need to convert
* bytes: convert bytes
*
* return: convert integer number
********************************************************/
uint64_t hexs_to_dec_lsbyte(const uint8_t *p_hex, uint8_t bytes)
{
		uint64_t dec = 0;
	uint8_t i;
	/* because return is 64bit, so convert bytes <= 8 */
	if (bytes > 8)
		return 0;
	
	for (i = 0; i < bytes; i++) {
		dec |= ((uint64_t)p_hex[i]) << (i<<3);
	}

	return dec;
}

/********************************************************
* 64 bits integer convert to BCD code
********************************************************/
void dec64_to_bcd(uint64_t dec, uint8_t *p_bcd, uint8_t bcds)
{
	int8_t i ;
	for (i = bcds-1; i >= 0; i--) {
		uint8_t temp = dec % 100;
		p_bcd[i] = ((temp/10)<<4) | (temp%10);
		dec /= 100;
	}
}

/********************************************************
* 16 bits integer convert to BCD code
********************************************************/
void dec16_to_bcd(uint16_t dec, uint8_t *p_bcd, uint8_t bcds)
{
	uint8_t temp;
	int i;
	for (i = bcds-1; i >= 0; i--) {
		 temp = dec % 100;
		p_bcd[i] = ((temp/10)<<4) | (temp%10);
		dec /= 100;
	}
}

/********************************************************
* dir: direction for first index of byte
* p_data: pointer to first byte of array
* bit: location of get bit
*
* return: bit data, 0 or 1
********************************************************/
uint8_t get_array_bit(byte_dir_t dir, const uint8_t *p_data, uint8_t bit)
{
	uint8_t bit_get;
	if (dir == e_dir_msbit)
		bit_get = (0x80 >> (bit&0x07));
	else
		bit_get = (0x01 << (bit&0x07));

	return p_data[bit>>3] & bit_get;
}


