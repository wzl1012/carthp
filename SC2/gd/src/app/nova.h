#ifndef _NOVA_H_
#define _NOVA_H_

typedef enum {
	e_nova_liw,
	e_nova_pmc,
	e_nova_without
} nova_synch_t;
typedef enum
{
	e_nova_not,
	e_nova_yes
} nova_mode_t;
typedef enum
{
	e_nova_13tag,
	e_nova_null
} nova_clone_t;

typedef struct
{
	modulation_t modu; // bit 10 is '0', use Biphase. bit 10 is'1', use Manchester.
	uint8_t periods;
	nova_synch_t synch;
	nova_mode_t mode;
	nova_clone_t clone;
	uint8_t a_words[8][2];		// LSBit-->MSBit
	uint8_t a_config[2];		// LSBit-->MSBit
	uint8_t a_psw[2];
} nova_tag_t;

extern nova_tag_t nova_tag1;
extern nova_tag_t nova_tag_copy;

uint8_t nova_find_tag(nova_tag_t *p_tag);
uint8_t nova_copy_em4100(nova_tag_t *p_nova, const em4100_tag_t *p_em4100);
uint8_t nova_keypad_interaction(nova_tag_t *p_tag);


#endif

