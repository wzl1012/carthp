#ifndef _SK_HITAG2_H_
#define _SK_HITAG2_H_

extern uint8_t sk_uid[4];

uint8_t sk_check_ori(uint8_t code, const uint8_t *ckdata, uint8_t reverse_flag);
uint8_t Fccrack46(uint8_t code, uint8_t *crkdata, uint8_t *key);

#endif


