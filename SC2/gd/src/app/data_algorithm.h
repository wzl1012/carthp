#ifndef _DATA_ALGORITHM_H_
#define _DATA_ALGORITHM_H_

uint8_t equal_byte(const uint8_t *p_to, const uint8_t byte, uint8_t bytes);
uint8_t equal_bytes(const uint8_t *p_to, const uint8_t *p_from, uint8_t bytes);
uint8_t xor_bytes(const uint8_t *p_data, uint8_t bytes);
void copy_bytes(uint8_t *p_to, const uint8_t *p_from, uint8_t bytes);
void copy_msbits(uint8_t *p_to, uint8_t offset_to, const uint8_t *p_from, uint8_t offset_from, uint8_t bits);
void copy_lsbits(uint8_t *p_to, uint8_t offset_to, const uint8_t *p_from, uint8_t offset_from, uint8_t bits);
void reverse_sequence(uint8_t *p_data, const uint8_t bytes);
uint8_t reverse_byte(uint8_t byte);
uint16_t reverse_word(uint16_t word);
uint8_t calculate_msbit_parity(const uint8_t *p_data, uint8_t bits);
uint64_t hexs_to_dec_msbyte(const uint8_t *p_hex, uint8_t bytes);
uint64_t hexs_to_dec_lsbyte(const uint8_t *p_hex, uint8_t bytes);
void dec64_to_bcd(uint64_t dec, uint8_t *p_bcd, uint8_t bcds);
void dec16_to_bcd(uint16_t dec, uint8_t *p_bcd, uint8_t bcds);
uint8_t get_array_bit(byte_dir_t dir, const uint8_t *p_data, uint8_t bit);


#endif

