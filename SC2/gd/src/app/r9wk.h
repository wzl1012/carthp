#ifndef _R9WK_H_
#define _R9WK_H_
//#include "gd32f10x.h"
#include <stdint.h>
typedef enum
{
	e_r9wk_not,
	e_r9wk_crc_ok,
	e_r9wk_crc_ng
} r9wk_mode_t;
typedef struct
{
	r9wk_mode_t mode;
	uint8_t start;
	uint8_t a_id[10];
	uint8_t stop;
	uint8_t end[2];
} r9wk_tag_t;
typedef struct
{
	uint8_t para[9];
	uint8_t ok_flag;
} tpx1_tag_t;
typedef struct
{
	uint8_t header;
	uint8_t a_id[8];
	uint8_t a_crc[2];
	uint8_t a_trailer[3];
	uint8_t is_hdx;
	uint8_t code_national[6];
	uint8_t code_country[2];
} hdx_tag_t;


extern r9wk_tag_t r9wk_tag_ori;
extern r9wk_tag_t r9wk_tag_copy;

uint8_t r9wk_find_tag(r9wk_tag_t *p_tag);
uint8_t r9wk_keypad_interaction(r9wk_tag_t *p_tag);
uint8_t hdx_renew_code(hdx_tag_t *p_hdx);
uint8_t hdx_find_tag(hdx_tag_t *p_hdx);
uint8_t hdx_display_tag(uint8_t num, hdx_tag_t *p_hdx);

uint8_t tpx1_copy_r9wk(const r9wk_tag_t *p_r9wk);
uint8_t ts4c_backdoor_write_byte(const uint8_t addr, const uint8_t byte);
uint8_t ts4c_backdoor_read_byte(const uint8_t addr, uint8_t *p_rd);
uint8_t ts4c_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);

uint8_t ts4c_copy_r9wk(const uint8_t *p_wr);
uint8_t ts0c_copy_r9wk(const uint8_t *p_wr);

void tag4c_fast_read_init(void);
uint8_t tag4c_fast_read(void);
void ts42_init_tag_init(void);
uint8_t ts42_init_tag(void);

#endif

