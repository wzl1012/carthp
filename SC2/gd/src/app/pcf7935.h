#ifndef _PCF7935_H_
#define _PCF7935_H_
#include <stdint.h>
typedef enum
{
	e_pcf7935_not,
	e_pcf7935_yes
} pcf7935_mode_t;
typedef struct
{
	pcf7935_mode_t mode;
	uint8_t a_id[16];
} pcf7935_tag_t;

extern pcf7935_tag_t pcf7935_tag1;

uint8_t pcf7935_find_tag(pcf7935_tag_t *p_tag);
uint8_t pcf7935_keypad_interaction(pcf7935_tag_t *p_tag);


#endif

