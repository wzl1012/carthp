#ifndef _TSTAG_H_
#define _TSTAG_H_
#include "pcf7936.h"

extern ts06_tag_t ts06_tag1;

uint8_t ts06_init_id(uint8_t *p_id, uint8_t code);
uint8_t ts06_copy_pcf7936(ts06_tag_t *p_tag_wr, const pcf7936_tag_t *p_tag_rd);
uint8_t ts06_find_tag(ts06_tag_t *p_tag);
bool ts06_keypad_interaction(ts06_tag_t *p_tag);



#endif

