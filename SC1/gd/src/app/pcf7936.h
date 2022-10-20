#ifndef _PCF7936_H_
#define _PCF7936_H_
#include "headfile.h"
#define PCF7936_T_O					(8)
// Command Handling
#define PCF7936_T_WAIT_TR			(1300)					// Transponder response delay
#define PCF7936_T_WAIT_BS			(6000)//V2.25:(4000)	// Basestation response delay
#if _INIT_TS_TAG
#define PCF7936_T_PROG				(15)					// (8 * 615 / 1000)
#define PCF7936_T_IDLE_MAX			(10)					// ms 
#define PCF7936_CHARGE_TIME			(50)
#define PCF7936_T_RESET_MIN			(20)					// ms, LF Field Power On Reset setup time
#else
#define PCF7936_T_PROG				(40)					// (8 * 615 / 1000)
#define PCF7936_T_IDLE_MAX			(100)					// ms 
#define PCF7936_CHARGE_TIME			(70)
#define PCF7936_T_RESET_MIN			(100)					// ms, LF Field Power On Reset setup time
#endif
// LF Field Power On Reset
#define PCF7936_T_START				(80)					// Transponder initialization time
#define PCF7936_T_INIT				(225 * PCF7936_T_O)		// Transponder initialization time

// Read Only Mode
#define PCF7936_T_WAIT_SA			(320 * PCF7936_T_O)		// Timeout for START_AUTH command
#define PCF7936_T_WAIT_RO			(551 * PCF7936_T_O)		// Read Only Mode startup delay

#define EEP_TS46_SKRY				(0xFE0)


typedef enum
{
	e_ts06_not,
	e_ts06_yes
} ts06_mode_t;
typedef struct
{
	ts06_mode_t mode;
	uint8_t step;
	uint8_t groups;
	uint8_t tmcf_enc;
	modulation_t tmcf_dcs;
	uint8_t a_version[4];
	uint8_t a_id[4];
	uint8_t a_keys[12];			// MSByte-->LSBytea
	uint8_t a_sign[24][4];
	uint8_t a_page[8][4];		// sequence: 1 2 0 3 4 5 6 7
} ts06_tag_t;


enum
{
	e_pcf7936_psw = 0x10,
	e_pcf7936_cipher,
};

typedef enum
{
	e_power_off,
	e_wait,
	e_auth_half,
	e_authorized,
	e_read_all
} pcf7936_stage_t;
typedef enum
{
	e_pcf7936_not,
	e_pcf7936_yes,
	e_pcf7936_ts46_blank,
	e_pcf7936_cracked
} pcf7936_mode_t;

typedef enum
{
	defaultcartype=0,
	hongda,
	exc46,
	lastcartype
} cartype_t;
typedef enum
{
	nocrty=0,
	hitag2,
	hitag3
} crotymode_t;
typedef struct
{
	pcf7936_stage_t now_state;
	pcf7936_mode_t mode;
	uint8_t tmcf_enc;
	modulation_t tmcf_dcs;
	uint8_t a_page[8][4];
	uint8_t random[4];			// MSbit:  :LSBit ,hitag2:32bit,hitag2+:64bit
	uint8_t secret_key[6];		// MSbit:  :LSBit,hitag2:48bit,hitag2+:96bit
	uint8_t cartype;
} pcf7936_tag_t;


typedef struct
{
	uint8_t mode;
	modulation_t tmcf_dcs;
	uint8_t collect;		// number of valid colllect data
	uint8_t a_config[4];	// ts06 new tag config data
	uint8_t a_id[4];
	uint8_t a_sign[6][4];	// 3 groups signature, one group include 8 bytes
	uint8_t a_keys[12];		// MSByte-->LSBytea
} ts06new_tag_t;

extern pcf7936_tag_t pcf7936_tag1;
extern pcf7936_tag_t pcf7936_tag_temp;
extern uint8_t pcf7936_t_charge;
extern uint8_t g_pcf7991_p0_pcf7936;


extern ts06new_tag_t ts06new_tag1;

void pcf7936_off_to_wait(void);
void pcf7936_to_off(void);
void pcf7936_wr_msbits(const uint8_t *p_data, uint8_t bits, uint8_t stop_flag);
uint8_t pcf7936_auth_half(pcf7936_tag_t *p_tag);

uint8_t pcf7936_read_tag(pcf7936_tag_t *p_tag, uint8_t start_p, uint8_t end_p);
void pcf7936_clear_tag_flag(pcf7936_tag_t *p_tag);
uint8_t pcf7936_find_tag(pcf7936_tag_t *p_tag);
uint8_t pcf7936_keypad_interaction(pcf7936_tag_t *p_tag);

void pcf7936_fast_read_init(void);
void pcf7936_fast_read(pcf7936_tag_t *p_tag);

void ts06new_init_tag_init(void);
void ts06_pcf7936typeinit_tag(pcf7936_tag_t *p_tag/*,ic46type_t ictype,crotymode_t crotymode*/);
void ts06new_init_tag(pcf7936_tag_t *p_tag);
void ts46_init_tag_init(void);
void ts46_init_tag(pcf7936_tag_t *p_tag);
uint8_t ts06new_init_id(uint8_t *p_id, uint8_t code);
uint8_t ts06new_find_tag(ts06new_tag_t *p_tag, pcf7936_tag_t *p_tag_ori);
uint8_t ts06new_keypad_interaction(ts06new_tag_t *p_tag);
uint8_t tag_copy_pcf7936(ts06_tag_t *p_ts06, pcf7936_tag_t *p_ts46, pcf7936_tag_t *p_tag_rd);

uint8_t pcf7936_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);


#endif

