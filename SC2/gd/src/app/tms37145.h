#ifndef _TMS37145_H_
#define _TMS37145_H_
#include "r9wk.h"
enum
{
	e_page_unlock = 0x10,
	e_page_lock
};

typedef enum
{
	e_tms37145_not,
	e_tms37145_pwm,
	e_tms37145_ppm,
	e_tms37145_b9wk,
	e_b9wk			// 4d tag
} tms37145_mode_t;

typedef struct
{
	uint8_t read_len;
	uint8_t crack_flag;
	uint8_t crack_type;
	tms37145_mode_t mode;
	uint8_t page1[2];
	uint8_t page2[2];
	uint8_t page3[5];		// [0]-->[3] == MSByte-->LSByte
	uint8_t page4[6];		// [0]-->[4] == MSByte-->LSByte
	uint8_t signature[4][3];
	uint8_t page8[5];
	uint8_t page18;
	uint8_t page30[2];
} tms37145_tag_t;
typedef struct
{
	uint8_t page1;
	uint8_t page2;
	uint8_t page3[4];		// [0]-->[3] == LSByte-->MSByte
	uint8_t page4[5];		//[0]-->[4] ==  LSByte-->MSByte
	uint8_t page5;
	uint8_t page6;
} ts4d_tag_t;

extern tms37145_tag_t tms37145_tag1;
extern tms37145_tag_t tms37145_tag_copy;

typedef struct
{
	uint8_t read_len;
	uint8_t psw;
	uint8_t para[4];
} tpx2_tag_t;



uint16_t tms37145_calc_crc16(const uint8_t *p_data, uint8_t bytes);
void tms37145_pwm_wr_lsbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms);
uint8_t tms37145_pwm_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd);
uint8_t tms37145_ppm_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd);
uint8_t tms37145_pwm_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd);
uint8_t tms37145_ppm_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd);
uint8_t tms37145_pwmtime_wr_ls_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd);
uint8_t tms37145_pwmtime_wr_ls_crc_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd);

uint8_t tms37145_crack_dst40(tms37145_tag_t *p_tag);
bool tms37145_find_tag(tms37145_tag_t *p_tag);
bool tms37145_keypad_interaction(tms37145_tag_t *p_tag);

void tag64_fast_read_init(void);
uint8_t tag64_fast_read(void);
void tms37145_get_data_init(void);
uint8_t tms37145_get_data(void);
void tms37145_fast_read_init(void);
uint8_t tms37145_fast_read(void);
void ts32_init_tag_init(void);
uint8_t ts32_init_tag(void);
void ts8x_init_tag_init(void);
uint8_t ts86_init_tag63(void);
uint8_t ts85_init_tag4d(void);


uint8_t ts0dnew_copy_b9wk(const tms37145_tag_t *p_b9wk);
uint8_t ts0dnew_copy_r9wk(const r9wk_tag_t *p_r9wk);
void ts0d_init_tag63_1_init(void);
uint8_t ts0d_init_tag63_1(void);
void ts0d_init_tag63_2_init(void);
uint8_t ts0d_init_tag63_2(void);
void ts0d_init_tag63_init(void);
uint8_t ts0d_init_tag63(void);
void ts0d_init_tag4d_init(void);
uint8_t ts0d_init_tag4d(void);
void ts0d_init_tag64_init(void);
uint8_t ts0d_init_tag64(void);
void ts0d_unlock_tag_init(void);
uint8_t ts0d_unlock_tag(void);

uint8_t tpx2_copy_b9wk(const tms37145_tag_t *p_b9wk);
uint8_t ts0d_copy_b9wk(const tms37145_tag_t *p_b9wk);
uint8_t ts804d_copy_b9wk(const tms37145_tag_t *p_b9wk);
uint8_t ys80_copy_b9wk(const tms37145_tag_t *p_b9wk);
uint8_t ys01_copy_b9wk(const tms37145_tag_t *p_b9wk);
uint8_t ys01_copy_r9wk(const r9wk_tag_t *p_r9wk);

uint8_t b9wk60_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);
uint8_t b9wk64_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);
uint8_t tms37145_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);


#endif

