/********************************************************
* nova==tag T5
********************************************************/
// The RM pattern consists of 2 bits "0" 
// During the phase where the chip has its modulator "ON" (1.0 bit of LIW or 2.0 bit in PMC ), the transceiver has to send a bit "0". 

#include "headfile.h"

#define NOVA_T_RF					(8)
#define NOVA_T_WA					(3*8)
#define NOVA_T_WEE					(111*8)
#define NOVA_T_WEE_MAX				(57)					// ms
#define NOVA_T_INIT_HALF			(308 * NOVA_T_RF)
#define NOVA_T_INIT					(616 * NOVA_T_RF)		// actual measure 5200 us, here would less then 5200
#define NOVA_T_RD_DLY				(22 * NOVA_T_RF)		// actual measure

#define NOVA_LIW_MODU1				(1)
#define NOVA_LIW_ALL				(2)

#define NOVA_CMD_LOGIN				(0x30 >> 2)
#define NOVA_CMD_WR_PSW				(0x50 >> 2)
#define NOVA_CMD_WR_WORD			(0x60 >> 2)
#define NOVA_CMD_RD_CFG				(0x90 >> 2)
#define NOVA_CMD_RD_MEM				(0x00 >> 2)

#define NOVA_CFG_MODU				(0x20)
#define NOVA_CFG_SIZE				(0x10)
#define NOVA_CFG_RATE				(0x0C)
#define NOVA_CFG_MODE				(0x03)

nova_tag_t nova_tag1;
nova_tag_t nova_tag_copy;

void nova_off_to_read(void)
{
	pcf7991_active_driver();
	delay_1us(10);
	pcf7991_set_page(0, PCF7991_PAGE0_NOVA);
	delay_1us(10);
	pcf7991_read_tag();
	delay_1us(10);
	
	delay_1us(NOVA_T_INIT / 2);
	delay_1us(NOVA_T_INIT / 2 - 100);	// -100 is actual measure
}

void nova_to_off(void)
{
	pcf7991_inactive_driver();
	delay_1us(70*1000);
}

void nova_wr_msbits(const uint8_t *p_wr, const uint8_t bits, uint8_t periods)
{
	uint8_t i, byte=0;
	uint16_t half_period = (uint16_t)periods << 3;
	
	for (i = 0; i < bits; i++) {
		pcf7991_driver_on();
		if ((i % 8) == 0)
			byte = *p_wr++;
		
		timer1_stop();
		TCNT1 = 65535-(2 * 2);			// see assembler code
		timer1_start_8();

		while (TCNT1 < (16 * 2));
		
		if ((byte & 0x80) == 0)
			pcf7991_driver_off();		
		while (TCNT1 < half_period);
		
		pcf7991_driver_on();		
		while (TCNT1 < (half_period*2 - 4));

		byte <<= 1;
	}

	timer1_stop();
}

uint8_t nova_find_liw(uint8_t periods, uint8_t liw_flag)
{
		uint8_t result;
	uint16_t tcnt_pre=0;
		const uint16_t quarter_period = ((uint16_t)periods << 2);
	enum {
		miss_high,
		miss_low_05, high_05, low_20, high_10, low_10,
		find_liw, s_exit
	} state;
	switch (periods) {
		case 64:
		case 40:
		case 32:
			break;
		default:
			return FALSE;
	}
	switch (liw_flag) {
		case NOVA_LIW_MODU1:
		case NOVA_LIW_ALL:
			break;
		default:
			return FALSE;
	}
	
	
	timer1_stop();
	timer1_clear_ov();
	TCNT1 = /*65536 - */((uint16_t)periods<<8);		// (16 / 5) times
	timer1_start_8();

	result = FALSE;
	state = miss_high;
	while ((state != s_exit) && (timer1_ov_flag == 0)) {
		switch (state) {
			case miss_high:
				if (pcf7991_dout_level == 0)
					state = miss_low_05;
				break;
			case miss_low_05:
				if (pcf7991_dout_level) {
					tcnt_pre = TCNT1;
					state = high_05;
				}
				break;
			case high_05:
				if (pcf7991_dout_level == 0) {
					if (((TCNT1-tcnt_pre)>=quarter_period) && ((TCNT1-tcnt_pre)<=(quarter_period*3))){
						tcnt_pre = TCNT1;
						state = low_20;
					}
					else
						state = miss_low_05;
				}
				break;
			case low_20:
				if (pcf7991_dout_level) {
					if (((TCNT1-tcnt_pre)>=(quarter_period*6)) && ((TCNT1-tcnt_pre)<=(quarter_period*10))){
						tcnt_pre = TCNT1;
						if (liw_flag == NOVA_LIW_ALL)
							state = high_10;
						else
							state = find_liw;
					}
					else
						state = s_exit;
				}
				break;
			case high_10:
				if (pcf7991_dout_level == 0) {
					if (((TCNT1-tcnt_pre)>=(quarter_period*3)) && ((TCNT1-tcnt_pre)<=(quarter_period*5))){
						tcnt_pre = TCNT1;
						state = low_10;
					}
					else
						state = s_exit;
				}
				break;
			case low_10:
				if ((TCNT1-tcnt_pre)>=(quarter_period*3)) {
					if (pcf7991_dout_level == 0) {
						while ((TCNT1-tcnt_pre)<(quarter_period*4));
						state = find_liw;
					}
					else
						state = s_exit;
				}
				break;
			case find_liw:
				result = TRUE;
				state = s_exit;
				break;
			default :
				state = s_exit;
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	return result;
}

uint8_t nova_find_ack(uint8_t periods, uint8_t times)
{
		uint8_t result;
	uint16_t tcnt_pre=0;
	const uint16_t quarter_period = ((uint16_t)periods << 2);
	enum {
		miss_high, 
		miss_low1_05, high2_05, low3_15, high4_05, low5_15, high6_05,
		find_ack, s_exit
	} state;
	switch (periods) {
		case 64:
		case 40:
		case 32:
			break;
		default:
			return FALSE;
	}

	
	timer1_stop();
	timer1_clear_ov();
	TCNT1 =/* 65536 - */((uint16_t)periods<<8);		// (16 / 5) times
	timer1_start_8();

	result = FALSE;
	state = miss_high;
	while ((state != s_exit) && (timer1_ov_flag == 0)) {
		switch (state) {
			case miss_high:
				if (pcf7991_dout_level == 0)
					state = miss_low1_05;
				break;
			case miss_low1_05:
				if (pcf7991_dout_level) {
					tcnt_pre = TCNT1;
					state = high2_05;
				}
				break;
			case high2_05:
				if (pcf7991_dout_level == 0) {
					if (((TCNT1-tcnt_pre)>=quarter_period) && ((TCNT1-tcnt_pre)<=(quarter_period*3))){
						tcnt_pre = TCNT1;
						state = low3_15;
					}
					else
						state = s_exit;
				}
				break;
			case low3_15:
				if (pcf7991_dout_level) {
					if (((TCNT1-tcnt_pre)>=(quarter_period*4)) && ((TCNT1-tcnt_pre)<=(quarter_period*8))){
						tcnt_pre = TCNT1;
						state = high4_05;
					}
					else
						state = s_exit;
				}
				break;
			case high4_05:
				if (pcf7991_dout_level == 0) {
					if (((TCNT1-tcnt_pre)>=quarter_period) && ((TCNT1-tcnt_pre)<=(quarter_period*3))){
						tcnt_pre = TCNT1;
						state = low5_15;
					}
					else
						state = s_exit;
				}
				break;
			case low5_15:
				if (pcf7991_dout_level) {
					if (((TCNT1-tcnt_pre)>=(quarter_period*4)) && ((TCNT1-tcnt_pre)<=(quarter_period*8))){
						tcnt_pre = TCNT1;
						state = high6_05;
					}
					else
						state = s_exit;
				}
				break;
			case high6_05:
				if (((TCNT1-tcnt_pre)>quarter_period) || (pcf7991_dout_level == 0)) {
					if (pcf7991_dout_level) {
						while ((TCNT1-tcnt_pre)<(quarter_period*2));
						state = find_ack;
					}
					else
						state = s_exit;
				}
				break;
			case find_ack:
				--times;
				if (times > 0) {
					TCNT1 = /*65536 - */((uint16_t)periods<<8); 	// (16 / 5) times
					state = miss_high;
				}
				else {
					result = TRUE;
					state = s_exit;
				}
				break;
			default :
				state = s_exit;
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	return result;
}

uint8_t nova_detect_periods(uint8_t *p_periods)
{
	uint8_t result, period;

	period = 64;
	nova_off_to_read();
	result = nova_find_liw(64, NOVA_LIW_ALL);
	nova_to_off();
	if (!result) {
		period = 32;
		nova_off_to_read();
		result = nova_find_liw(32, NOVA_LIW_ALL);
		nova_to_off();
	}
	if (!result) {
		period = 40;
		nova_off_to_read();
		result = nova_find_liw(40, NOVA_LIW_ALL);
		nova_to_off();
	}
	
	if (result)
		*p_periods = period;

	return result;
}

/********************************************************
* MSBit --> LSBit
********************************************************/
uint8_t nova_write_word(uint8_t addr, uint16_t word, nova_tag_t *p_tag)
{
	uint8_t i, result, login_flag;
	uint8_t a_cmd[2], a_wr_data[5];

	if (addr < 8)
		login_flag = TRUE;
	else if (addr == 8)
		login_flag = FALSE;
	else
		return FALSE;
	
	a_cmd[0] = NOVA_CMD_WR_WORD;
	a_cmd[1] = NOVA_CMD_LOGIN;
	addr <<= 4;
	if (calculate_msbit_parity(&addr, 4) == 0x01)
		addr |= 0x08;
	em4170_word_to_array(word, a_wr_data);

	result = FALSE;
	for (i = 0; (!result) && (i < 2); i++) {
		nova_off_to_read();
		if (!login_flag) {
			if (nova_find_liw(p_tag->periods, NOVA_LIW_MODU1)) {
				pcf7991_write_tag_n(0);
				nova_wr_msbits(&a_cmd[1], 6, p_tag->periods);
				nova_wr_msbits(p_tag->a_psw, 16, p_tag->periods);
				pcf7991_set_page(0, PCF7991_PAGE0_NOVA);
				pcf7991_read_tag();
				delay_1us(NOVA_T_RD_DLY);
				timer1_delay_us((uint16_t)p_tag->periods * 8 * 5);
				if (nova_find_ack(p_tag->periods, 2)) {
					login_flag = TRUE;
				}
			}
		}

		if (login_flag && (nova_find_liw(p_tag->periods, NOVA_LIW_MODU1))) {
			pcf7991_write_tag_n(0);
			nova_wr_msbits(a_cmd, 6, p_tag->periods);
			nova_wr_msbits(&addr, 5, p_tag->periods);
			nova_wr_msbits(&a_wr_data[0], 5, p_tag->periods);
			nova_wr_msbits(&a_wr_data[1], 5, p_tag->periods);
			nova_wr_msbits(&a_wr_data[2], 5, p_tag->periods);
			nova_wr_msbits(&a_wr_data[3], 5, p_tag->periods);
			nova_wr_msbits(&a_wr_data[4], 5, p_tag->periods);
			pcf7991_set_page(0, PCF7991_PAGE0_NOVA);
			pcf7991_read_tag();
			delay_1us(NOVA_T_RD_DLY);
			timer1_delay_us((uint16_t)p_tag->periods * NOVA_T_WA - 10);
			timer1_delay_us((uint16_t)p_tag->periods * 8 * 5);
			if (nova_find_ack(p_tag->periods, 2)) {
				result = TRUE;
			}
			delay_1us(NOVA_T_WEE_MAX * 2*1000);
		}
		nova_to_off();
	}

	return result;
}

uint8_t nova_read_data(uint8_t cmd, nova_tag_t *p_tag)
{
	uint8_t result, rd_bits;
	uint8_t *p_rd;
	uint8_t (*p_fun)(uint8_t , uint8_t *, uint8_t , uint8_t);

	switch (cmd) {
		case NOVA_CMD_RD_CFG:
			rd_bits = 16;
			p_rd = p_tag->a_config;
			break;
		case NOVA_CMD_RD_MEM:
			rd_bits = 128;
			p_rd = &(p_tag->a_words[0][0]);
			break;
		default:
			return FALSE;
	}
	if (p_tag->modu == e_modu_mc)
		p_fun = pcf7991_decode_mc;
	else if (p_tag->modu == e_modu_bp)
		p_fun = pcf7991_decode_bp;
	else
		return FALSE;

	result = FALSE;
	nova_off_to_read();
	if (nova_find_liw(p_tag->periods, NOVA_LIW_MODU1)) {
		pcf7991_write_tag_n(0);
		nova_wr_msbits(&cmd, 6, p_tag->periods);
		pcf7991_set_page(0, PCF7991_PAGE0_NOVA);
		pcf7991_read_tag();
		delay_1us(NOVA_T_RD_DLY);
		timer1_delay_us((uint16_t)p_tag->periods * 8 * 5);
		if (nova_find_ack(p_tag->periods, 2))
			if (p_fun(p_tag->periods, p_rd, rd_bits, e_dir_msbit))
				result = nova_find_liw(p_tag->periods, NOVA_LIW_ALL);
	}
	nova_to_off();

	return result;
}

uint8_t nova_find_tag(nova_tag_t *p_tag)
{
		uint8_t result;
	p_tag->mode = e_nova_not;
	p_tag->clone = e_nova_null;

	if (!nova_detect_periods(&(p_tag->periods)))
		return FALSE;

	// decide modulation, get config data
	p_tag->modu = e_modu_mc;
	result = nova_read_data(NOVA_CMD_RD_CFG, p_tag);
	if ((!result) || ((p_tag->a_config[1] & 0x20) == 0x00)) {
		p_tag->modu = e_modu_bp;
		result = nova_read_data(NOVA_CMD_RD_CFG, p_tag);
	}

	// get word0 - word7
	if (result) {
		result = nova_read_data(NOVA_CMD_RD_MEM, p_tag);
	}
	
	if (result) {
		p_tag->mode = e_nova_yes;
		switch (p_tag->a_config[1] & NOVA_CFG_MODE) {
			case 0:
				p_tag->synch = e_nova_liw;
				break;
			case 2:
				p_tag->synch = e_nova_pmc;
				break;
			case 1:
				p_tag->synch = e_nova_without;
				if (em4100_find_tag(&em4100_tag1, PCF7991_PAGE0_EM4100))
					p_tag->clone = e_nova_13tag;
				break;
			default:
				break;
		}
	}

	return result;
}

uint8_t nova_copy_nova(nova_tag_t *p_to, const nova_tag_t *p_from)
{
	uint8_t i, result;
	uint16_t word;

	if (!nova_detect_periods(&(p_to->periods)))
		return FALSE;

	// decide modulation, get config data
	p_to->modu = e_modu_mc;
	result = nova_read_data(NOVA_CMD_RD_CFG, p_to);
	if ((!result) || ((p_to->a_config[1] & 0x20) == 0x00)) {
		p_to->modu = e_modu_bp;
		result = nova_read_data(NOVA_CMD_RD_CFG, p_to);
	}
	if (!result) 
		return FALSE;

	if (p_to->a_config[0] || (p_to->a_config[1] & 0xC0))
		return FALSE;

	result = FALSE;
	for (i = 0; i < 8; i++) {
		word = (uint16_t)p_from->a_words[i][0] << 8;
		word |= (uint16_t)p_from->a_words[i][1];
		word = reverse_word(word);
		if (!nova_write_word(i, word, p_to))
			break;
	}
	if (i == 8) {
		p_to->a_psw[0] = 0;
		p_to->a_psw[1] = 0;
		word = (uint16_t)p_from->a_config[0] << 8;
		word |= (uint16_t)p_from->a_config[1];
		word = reverse_word(word);
		result = nova_write_word(8, word, p_to);
	}

	return result;
}

uint8_t nova_copy_em4100(nova_tag_t *p_nova, const em4100_tag_t *p_em4100)
{
	uint8_t i, result;
	uint16_t word;

	if (!nova_detect_periods(&(p_nova->periods)))
		return FALSE;

	// decide modulation, get config data
	p_nova->modu = e_modu_mc;
	result = nova_read_data(NOVA_CMD_RD_CFG, p_nova);
	if ((!result) || ((p_nova->a_config[1] & 0x20) == 0x00)) {
		p_nova->modu = e_modu_bp;
		result = nova_read_data(NOVA_CMD_RD_CFG, p_nova);
	}
	if (!result) 
		return FALSE;

	if (p_nova->a_config[0] || (p_nova->a_config[1] & 0xC0))
		return FALSE;

	result = FALSE;
	for (i = 0; i < 4; i++) {
		word = (uint16_t)(p_em4100->a_data[i*2]) << 8;
		word |= (uint16_t)(p_em4100->a_data[i*2+1]);
		word = reverse_word(word);
		if (!nova_write_word(i, word, p_nova))
			break;
	}
	if (i == 4) {
		word = 0x0000;
		/* modulation */ 
		if ((p_em4100->mode == e_em4100_mc32) || (p_em4100->mode == e_em4100_mc64))
			word |= (0x8000 >> 10);
		/* memory size, 64bits */
		word |= (0x8000 >> 11);
		/* bit rate */
		if ((p_em4100->mode == e_em4100_mc32) || (p_em4100->mode == e_em4100_bp32))
			word |= (0x8000 >> 13);
		/*  without liw */
		word |= (0x8000 >> 15);
		word = reverse_word(word);
		p_nova->a_psw[0] = 0;
		p_nova->a_psw[1] = 0;
		result = nova_write_word(8, word, p_nova);
	}

	return result;
}

uint8_t nova_keypad_interaction(nova_tag_t *p_tag)
{
		uint8_t key;
	enum {
		s_show_0123, s_show_4567,
		s_show_copy_tag, s_copying, s_copy_ok, s_copy_ng,
		s_exit
	} state;
	if (p_tag->mode == e_nova_not)
		return FALSE;

	state = s_show_0123;
	while (state != s_exit) {
		switch (state) {
			case s_show_0123:
				lcd_clear_screen();
				lcd_print_str(0, 0, str_t5_tag, 6);
				if (p_tag->clone == e_nova_13tag)
					lcd_print_str(6, 0, str_13_mode, 8);
				lcd_print_hex(30, 40,200,24,24, p_tag->a_config, 2);
				delay_1us(T_DISPLAY*1000);

				lcd_print_hex(30, 40,200,24,24, &(p_tag->a_words[0][0]), 8);
				lcd_print_str(0, 1, str_read_r, 6);
				lcd_print_str(8, 1, str_copy_c, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_show_4567;
				else
					state = s_show_copy_tag;
				break;
			case s_show_4567:
				lcd_print_hex(30, 40,200,24,24, &(p_tag->a_words[4][0]), 8);
				lcd_print_str(0, 1, str_exit_r, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_show_copy_tag;
				break;
			case s_show_copy_tag:
				lcd_print_insert();
				lcd_print_str(8, 0, str_t5_tag, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_copying;
				break;
			case s_copying:
				lcd_print_copying();
				if (nova_copy_nova(&nova_tag_copy, p_tag))
					state = s_copy_ok;
				else
					state = s_copy_ng;
				break;
			case s_copy_ok:
			case s_copy_ng:
				lcd_clear_screen();
				if (state == s_copy_ok)
					lcd_print_str(0, 0, str_copy_ok, 8);
				else
					lcd_print_str(0, 0, str_copy_ng, 8);
				lcd_print_str(0, 1, str_exit_r, 6);
				lcd_print_str(8, 1, str_copy_c, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_show_copy_tag;
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return TRUE;
}



