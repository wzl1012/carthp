#ifndef _ATMEL_TAG_H_
#define _ATMEL_TAG_H_


typedef struct
{
	uint8_t a_wr[11];		// [0]-->[10] == MSByte-->LSByte
	uint8_t bits;
} at5577_wr_data_t;

typedef enum
{
	e_fbl,
	e_llr,
	e_lzr,
	e_oof
} at5577_protocol_t;

uint8_t at5577_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);



#if 0
#define E5561_T_SGAP				(150)
#define E5561_T_WGAP				(110)
#define E5561_T_BIT0				(24 * 8)
#define E5561_T_BIT1				(56 * 8)
#define E5561_T_EOT					(64 * 8)	//end of transmission
#else
#define E5561_T_SGAP				(100)
#define E5561_T_WGAP				(100)
#define E5561_T_BIT0				(134)
#define E5561_T_BIT1				(390)
#define E5561_T_EOT					(64 * 8)	//end of transmission
#endif

#define E5561_T_ACCESS				(900)
/* programming time is 16 ms, +1 ensure stable program */
#define E5561_T_PROGRAM				(16 + 1)
#define E5561_T_AUTH				(6)

#define E5561_OPCODE_RW				(0x01)
#define E5561_OPCODE_CRYPTO			(0x02)
#define E5561_OPCODE_STOP			(0x03)

typedef enum
{
	e_e5561_not,
	e_e5561_yes
} e5561_mode_t;

typedef struct
{
	e5561_mode_t mode;
	uint8_t a_block[5][4];		// LSBit-->MSBit
	uint8_t period;
	uint8_t termi;
	uint8_t id_bits;
	modulation_t modu;
} e5561_tag_t;

extern e5561_tag_t e5561_tag1;

uint8_t e5561_find_tag(e5561_tag_t *p_tag);
uint8_t e5561_keypad_interaction(e5561_tag_t *p_tag);
uint8_t e5561_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);

#endif

