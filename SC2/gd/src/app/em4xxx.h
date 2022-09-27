#ifndef _EM4XXX_H_
#define _EM4XXX_H_
#include <stdint.h>
typedef enum
{
	e_em4100_not,
	e_em4100_mc32,	//manchester, 32period
	e_em4100_mc64,	//manchester, 64period
	e_em4100_bp32,	//bi-phase, 32period
	e_em4100_bp64	//bi-phase, 64period
} em4100_mode_t;
typedef struct
{
	em4100_mode_t mode;
	uint8_t a_data[8];		// MBit-->LSBit
	uint8_t a_user[5];
} em4100_tag_t;

extern em4100_tag_t em4100_tag1;

uint8_t em4100_find_tag(em4100_tag_t *p_tag, uint8_t page0);
uint8_t em4100_keypad_interaction(em4100_tag_t *p_tag);
uint8_t em4102_display_tag(uint8_t num, const em4100_tag_t *p_tag);


uint8_t em4305_copy_em4100(const em4100_tag_t *p_em4100);
uint8_t em4305_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);


////////////////////////////////////////////////////////////////////////

#define EM4170_CMD_ID_MODE			(0x30)
#define EM4170_CMD_UM_MODE_1		(0x50)
#define EM4170_CMD_AUTHEN			(0x60)
#define EM4170_CMD_WR_WORD			(0xA0)
#define EM4170_CMD_SEND_PIN			(0x90)
#define EM4170_CMD_READ_ALL			(0xC0)
#define EM4170_CMD_UM_MODE_2		(0xF0)

enum
{
	e_copy_id = 0x01,
	e_copy_um1 = 0x02,
	e_copy_um2 = 0x04,
	e_copy_pin = 0x08,
	e_copy_crypt = 0x10
};

typedef enum
{
	e_no_em4170,
	e_yes_em4170
} em4170_mode_t;
typedef struct
{
	em4170_mode_t mode;
	uint8_t crypt_ori;
	uint8_t a_id[4];
	uint8_t a_um1[4];
	uint8_t a_um2[8];
	uint8_t key[12];
} em4170_tag_t;

extern em4170_tag_t em4170_tag1;

void em4170_word_to_array(uint16_t word, uint8_t *p_array);
uint8_t em4170_read_all(uint8_t *p_rd);
uint8_t em4170_find_tag(em4170_tag_t *p_tag);
uint8_t em4170_keypad_interaction(em4170_tag_t *p_tag);
uint8_t em4170_comm_handle(const uint8_t *p_ask, uint8_t *p_answer);
void em4170_fast_read_init(void);
uint8_t em4170_fast_read(void);
void ts48_init_tag_init(void);
void ts48_init_tag(void);


typedef struct
{
	uint8_t a_id[4];
	uint8_t a_um1[4];
	uint8_t a_um2[8];
} em4305_tag_t;

extern em4305_tag_t em4305_tag1;

#endif

