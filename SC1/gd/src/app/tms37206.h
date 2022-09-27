#ifndef _TMS37206_H_
#define _TMS37206_H_
#include "stm32f10x.h"


typedef enum
{
	e_ws21_not,
	e_ws21_yes
} tms37206_mode_t;


typedef struct
{
        uint8_t a_rd[12];
        uint8_t mode;
        uint8_t e_ws21_not;
        uint8_t e_ws21_yes;

} tms37206_tag_t;

extern tms37206_tag_t tms37206_tag1;

uint16_t TMS37206_calc_crc16(const uint8_t *p_data, uint8_t bytes);
void TMS37206_pwm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms);
uint8_t tms37206_pwm_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms ,uint8_t *p_rd);
uint8_t tms37206_pwm_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr,uint8_t recharge_ms, uint8_t *p_rd);
void tms37206_fast_read_init(void);
uint8_t tms37206_fast_read(void);
void tm37206_sign_wr(void);
void tms37206_eof(void);

bool tms37206_find_tag(tms37206_tag_t *p_tag);
bool tms37206_keypad_interaction(tms37206_tag_t *p_tag);

uint8_t tms37206_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);




#endif

