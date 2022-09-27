#ifndef _PCF7953_H_
#define _PCF7953_H_
#include "headfile.h"

enum
{
	e_pcf7953_psw = 0x10,
	e_pcf7953_cipher,
};
typedef enum
{
	e_pcf7953_not,
	e_pcf7953_yes,
	e_pcf7953_ts46_blank,
	e_pcf7953_cracked
} pcf7953_mode_t;

typedef struct
{
	pcf7936_stage_t now_state;
	pcf7953_mode_t mode;
	uint8_t tmcf_enc;
	modulation_t tmcf_dcs;
	uint8_t a_page[8][4];
	uint8_t random[8];			// MSbit:  :LSBit ,hitag2:32bit,hitag2+:64bit
	uint8_t secret_key[12];		// MSbit:  :LSBit,hitag2:48bit,hitag2+:96bit
	uint8_t cartype;
	//uint8_t hitagmode;
} pcf7953_tag_t;

extern pcf7953_tag_t pcf7953_tag;
extern uint64_t state_7953;
void pcf7953_init_tag(pcf7953_tag_t *p_tag);
uint8_t pcf7953_find_tag(pcf7953_tag_t *p_tag);
#endif
