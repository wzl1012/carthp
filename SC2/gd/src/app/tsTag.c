#include "headfile.h"

#define TS06_T_INIT					(70)		// Transponder initialization time
#define TS06_T_RESET_MIN			(100)
#define TS06_T_WAIT_BS				(15)		// ms

#define TS06_CMD_DEL_SEGMENT		(0x165A)
#define TS06_SEGMENT_DEL1			(0x1027)		// 0-7 page, 0-7 sign, total 16*4 = 64byte
#define TS06_SEGMENT_DEL2			(0x1069)		// 8-23 sign, total 16 * 4 = 64byte
#define TS06_SEGMENT_START			(0x1000)
#define TS06_SEGMENT_ID				(0x1008)

ts06_tag_t ts06_tag1;

void ts06_to_off(void)
{
	pcf7991_inactive_driver();
	delay_us(TS06_T_RESET_MIN*1000);
}

void ts06_off_to_wait(void)
{
	pcf7991_active_driver();
	delay_us(TS06_T_INIT*1000);
}

#define TS06_T_WRP				(7 * PCF7936_T_O)
#define TS06_T_LOG0				(20 * PCF7936_T_O)
#define TS06_T_LOG1				(29 * PCF7936_T_O)
#define TS06_T_STOP				(40 * PCF7936_T_O)
void ts06_wr_lsbits(const uint8_t *p_wr, uint8_t bits, bool stop_flag)
{
	uint8_t i, byte=0;

	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;

		pcf7991_driver_off();
		delay_us(TS06_T_WRP);
		
		pcf7991_driver_on();
		if (byte & 0x01)
			delay_us(TS06_T_LOG1 - TS06_T_WRP - 2);
		else
			delay_us(TS06_T_LOG0 - TS06_T_WRP - 2);

		byte >>= 1;
	}

	if (stop_flag) {
		pcf7991_driver_off();
		delay_us(TS06_T_WRP);
		
		pcf7991_driver_on();
		delay_us(TS06_T_STOP - TS06_T_WRP);
	}
}

/* LSBit first */
uint8_t ts06_wr_rd_frame(uint8_t *p_wr, uint8_t bits_wr, uint8_t *p_rd, uint8_t bits_rd)
{
	uint8_t result;
	uint8_t a_rd[5];

	result = FALSE;
	pcf7991_write_tag_n(TS06_PULSE_NUM);
	ts06_wr_lsbits(p_wr, bits_wr, TRUE);
	pcf7991_set_page(0x00, PCF7991_PAGE0_TS06);
	pcf7991_read_tag();
	delay_us(PCF7936_T_WAIT_TR);
	if (pcf7991_decode_miss_low()) {
		result = pcf7991_decode_mc(32, a_rd, 37, e_dir_lsbit);
	}

	if (result && ((a_rd[0]&0x1F) == 0x1F)) {
		copy_lsbits(p_rd, 0, a_rd, 5, bits_rd);
	}
	else
		result = FALSE;

	return result;
}

uint8_t ts06_get_version(uint8_t *p_read)
{
	uint8_t a_wr[2];

	a_wr[0] = 0xFF;
	a_wr[1] = 0x80;
	
	return ts06_wr_rd_frame(a_wr, 16, p_read, 32);
}

/* read 4 bytes */
/* zengwen name: read_TS06_data */
uint8_t ts06_read_eep(uint8_t addr,uint8_t index,uint8_t *p_read)
{
	uint8_t a_wr[2];

	if (addr == 0x20)
		a_wr[0] = (0x20 + index*4);
	else if (addr == 0xF0)
		a_wr[0] = (0xF0 + index*4);
	else
		a_wr[0] = (0x00 + index*4);
	a_wr[1] = (0x30);
	
	return ts06_wr_rd_frame(a_wr, 16, p_read, 32);
}

/* write 8 bytes */
/* zengwen name: write_ts06_data */
uint8_t ts06_write_eep(uint16_t addr, const uint8_t *p_wr)
{
	uint8_t i, result;
	uint8_t a_wr[12], a_rd[5];
	uint16_t sum16;

	a_wr[0] = ((uint8_t)addr);
	a_wr[1] = ((uint8_t)(addr>>8));
	copy_bytes(&a_wr[2], p_wr, 8);
	
	sum16 = 0xAA55;
	for(i = 0; i < 5; i++)
		sum16 += ((uint16_t)(a_wr[2*i + 1]<<8) | (uint16_t)a_wr[2*i]);
	
	a_wr[10]=(uint8_t)sum16;
	a_wr[11]=(uint8_t)(sum16>>8);

	result = FALSE;
	pcf7991_write_tag_n(TS06_PULSE_NUM);
	for (i = 0; i < 6; i++) {
		ts06_wr_lsbits(&a_wr[2*i], 16, TRUE);
		delay_us(2000);
	}
	pcf7991_set_page(0x00, PCF7991_PAGE0_TS06);
	pcf7991_read_tag();
	delay_us(300);
	if (pcf7991_decode_miss_low()) {
		result = pcf7991_decode_mc(32, a_rd, 37, e_dir_lsbit);
	}
	
	if (result) {
		result = FALSE;
		if ((a_rd[0]&0x1F) == 0x1F) {
			copy_lsbits(a_wr, 0, a_rd, 5, 32);
			if ((a_wr[0] == 0x55) && (a_wr[1] == 0xAA)
				 && (a_wr[2] == 0x4B) && (a_wr[3] == 0x4F))
				 result = TRUE;
		}
	}
	
	return result;
}

uint8_t ts06_check_fire_sum(ts06_tag_t *p_tag)
{
	uint8_t i, result;
	uint8_t a_wr[2], a_sum[4];
	uint16_t sum16;

	a_wr[0] = 0xF0;
	a_wr[1] = 0x80;

	result = FALSE;
	if (ts06_wr_rd_frame(a_wr, 16, a_sum, 32)) {
		for (sum16 = 0xAA55, i = 0; i < 24; i++) {
			sum16 += (((uint16_t)(p_tag->a_sign[i][1]) << 8) | ((uint16_t)(p_tag->a_sign[i][0])));
			sum16 += (((uint16_t)(p_tag->a_sign[i][3]) << 8) | ((uint16_t)(p_tag->a_sign[i][2])));
		}
		if (sum16 == (((uint16_t)a_sum[1] << 8) | ((uint16_t)a_sum[0])))
			if (sum16 == (((uint16_t)a_sum[3] << 8) | ((uint16_t)a_sum[2])))
				result = TRUE;
	}
	
	return result;
}

uint8_t ts06_delete_segment(uint16_t addr)
{
	uint8_t i, result;
	uint8_t a_wr[12], a_rd[5];
	uint16_t sum16;

	sum16 = 0xAA55 + TS06_CMD_DEL_SEGMENT + addr*4;

	a_wr[0] = (uint8_t)TS06_CMD_DEL_SEGMENT;
	a_wr[1] = (uint8_t)(TS06_CMD_DEL_SEGMENT>>8);
	a_wr[2] = (uint8_t)addr;
	a_wr[4] = a_wr[2];
	a_wr[6] = a_wr[2];
	a_wr[8] = a_wr[2];
	a_wr[3] = (uint8_t)(addr>>8);
	a_wr[5] = a_wr[3];
	a_wr[7] = a_wr[3];
	a_wr[9] = a_wr[3];
	a_wr[10] = (uint8_t)sum16;
	a_wr[11] = (uint8_t)(sum16>>8);

	result = FALSE;
	ts06_off_to_wait();
	pcf7991_write_tag_n(TS06_PULSE_NUM);
	for (i = 0; i < 6; i++) {
		ts06_wr_lsbits(&a_wr[2*i], 16, TRUE);
		delay_us(2000);
	}
	pcf7991_set_page(0x00, PCF7991_PAGE0_TS06);
	pcf7991_read_tag();
	delay_us(13*1000);
	if (pcf7991_decode_miss_low()) {
		result = pcf7991_decode_mc(32, a_rd, 37, e_dir_lsbit);
	}
	
	if (result) {
		result = FALSE;
		if ((a_rd[0]&0x1F) == 0x1F) {
			copy_lsbits(a_wr, 0, a_rd, 5, 32);
			if ((a_wr[0] == 0x55) && (a_wr[1] == 0xAA)
				 && (a_wr[2] == 0x4B) && (a_wr[3] == 0x4F))
				 result = TRUE;
		}
	}
	ts06_to_off();
	
	return result;
}

uint8_t ts06_init_id(uint8_t *p_id, uint8_t code)
{
	uint8_t result;
	uint8_t a_wr[8];

	a_wr[0] = reverse_byte(p_id[0]);
	a_wr[1] = reverse_byte(p_id[1]);
	a_wr[2] = reverse_byte(p_id[2]);
	a_wr[3] = reverse_byte(p_id[3]);
	if (code == e_modu_bp)
		a_wr[4] = 0xFF;
	else if (code == e_modu_mc)
		a_wr[4] = 0x7F;
	else
		return FALSE;
	a_wr[5] = 0xFF;
	a_wr[6] = 0xFF;
	a_wr[7] = 0xFF;
	
	result = FALSE;
	if (!ts06_delete_segment(TS06_SEGMENT_DEL1))
		;
	else if (!ts06_delete_segment(TS06_SEGMENT_DEL2))
		;
	else {
		ts06_off_to_wait();
		result = ts06_write_eep(TS06_SEGMENT_ID, a_wr);
		ts06_to_off();
	}
	
	return result;
}

/* copy 8 page data only */
uint8_t ts06_copy_pcf7936(ts06_tag_t *p_tag_wr, const pcf7936_tag_t *p_tag_rd)
{
		uint8_t i, j, result;
	uint8_t a_temp[4];
	// delete data
	if (!ts06_delete_segment(TS06_SEGMENT_DEL1))
		return FALSE;
	
	// updata page
	copy_bytes(&(p_tag_wr->a_page[0][0]), p_tag_rd->secret_key, 6);
	p_tag_wr->a_page[1][2] = 0x00;
	p_tag_wr->a_page[1][3] = 0x00;
	copy_bytes(p_tag_wr->a_page[2], p_tag_rd->a_page[0], 4);
	copy_bytes(p_tag_wr->a_page[3], p_tag_rd->a_page[3], 4);
	copy_bytes(&(p_tag_wr->a_page[4][0]), &(p_tag_rd->a_page[4][0]), 16);
	for (i = 0; i < 8; i++)
		for (j = 0; j < 4; j++)
			p_tag_wr->a_page[i][j] = reverse_byte(p_tag_wr->a_page[i][j]);

	// write 8 page
	ts06_off_to_wait();
	for (i = 0; i < 8; i += 2) {
		delay_us(TS06_T_WAIT_BS*1000);
		if (!ts06_write_eep(TS06_SEGMENT_START + 4*i, p_tag_wr->a_page[i]))
			break;
	}

	//checkout
	if (i == 8) {
		for (i = 0; i < 8; i++) {
			delay_us(TS06_T_WAIT_BS*1000);
			if (!ts06_read_eep(0x00, i, a_temp))
				break;
			else if (!equal_bytes(p_tag_wr->a_page[i], a_temp, 4))
				break;
		}
	}
	if (i == 8)
		result = TRUE;
	else
		result = FALSE;

	ts06_to_off();
	return result;
}

/* copy 8 page data and 24 sign */
uint8_t ts06_copy_ts06(const ts06_tag_t *p_tag_rd)
{
		uint8_t i, result;
	uint8_t a_temp[4];
	// delete data
	if (!ts06_delete_segment(TS06_SEGMENT_DEL1))
		return FALSE;
	else if (!ts06_delete_segment(TS06_SEGMENT_DEL2))
		return FALSE;
	

	/* write 8 page */
	result = FALSE;
	ts06_off_to_wait();
	for (i = 0; i < 8; i += 2) {
		delay_us(TS06_T_WAIT_BS*1000);
		if (!ts06_write_eep(TS06_SEGMENT_START + 4*i, p_tag_rd->a_page[i]))
			break;
	}
	if (i == 8) {
		for (i = 0; i < 8; i++) {
			delay_us(TS06_T_WAIT_BS*1000);
			if (!ts06_read_eep(0x00, i, a_temp))
				break;
			else if (!equal_bytes(p_tag_rd->a_page[i], a_temp, 4))
				break;
		}
	}
	
	/* write 24 sign */
	if (i == 8) {
		for (i = 0; i < 24; i += 2) {
			delay_us(TS06_T_WAIT_BS*1000);
			if (!ts06_write_eep(TS06_SEGMENT_START + 0x20 + 4*i, p_tag_rd->a_sign[i]))
				break;
		}
		if (i == 24) {
			for (i = 0; i < 24; i++) {
				delay_us(TS06_T_WAIT_BS*1000);
				if (!ts06_read_eep(0x20, i, a_temp))
					break;
				else if (!equal_bytes(p_tag_rd->a_sign[i], a_temp, 4))
					break;
			}
		}
	}
	if (i == 24)
		result = TRUE;

	ts06_to_off();
	return result;
}

void ts06_decide_mode(ts06_tag_t *p_tag)
{
	uint8_t i;

	for (i = 0; i < 4; i++)
		p_tag->a_id[i] = reverse_byte(p_tag->a_page[2][i]);
	
	if (p_tag->a_page[3][0] & 0x80)
		p_tag->tmcf_dcs = e_modu_bp;
	else
		p_tag->tmcf_dcs = e_modu_mc;

	if (equal_byte(&(p_tag->a_page[2][0]), 0xFF, 4))
		p_tag->step = 0;
	else if(equal_byte(&(p_tag->a_page[0][0]), 0xFF, 8))
		p_tag->step = 1;
	else
		p_tag->step = 2;

	for (i = 0, p_tag->groups = 0; i < 24; i += 2) {
		if (!equal_byte(&(p_tag->a_sign[i][0]), 0xFF, 8))
			p_tag->groups += 1;
	}

	p_tag->mode = e_ts06_yes;
}

uint8_t ts06_find_tag(ts06_tag_t *p_tag)
{
		uint8_t i, result;
	p_tag->mode = e_ts06_not;
	
	
	/* different between ts06 and tag46 */
	ts06_to_off();
	ts06_off_to_wait();
	result = ts06_get_version(p_tag->a_version);

	/*  get 24*4 byte sign data and check sum */
	if (result) {
		result = FALSE;
		for (i = 0; i < 24; i++) {
			delay_us(TS06_T_WAIT_BS*1000);
			if (!ts06_read_eep(0x20, i, p_tag->a_sign[i]))
				break;
		}
		if (i == 24) {
			delay_us(TS06_T_WAIT_BS*1000);
			result = ts06_check_fire_sum(p_tag);
		}
	}
	/* get 8*4 byte page data */
	if (result) {
		result = FALSE;
		for (i = 0; i < 8; i++) {
			delay_us(TS06_T_WAIT_BS*1000);
			if (!ts06_read_eep(0x00, i, p_tag->a_page[i]))
				break;
		}
		if (i == 8)
			result = TRUE;
	}
	ts06_to_off();
	
	if (result)
		ts06_decide_mode(p_tag);

	return result;
}

bool ts06_keypad_interaction(ts06_tag_t *p_tag)
{
		uint8_t key, i_group;
	uint8_t a_display[LCD_ROW_LENGTH];
		enum {
		s_waiting, s_show_crack, s_show_ori_copy, s_ori_copy,
		s_crack, s_show_skey, s_crack_copy,
		s_exit
	} state;
	if (p_tag->mode == e_ts06_not)
		return FALSE;


	state = s_waiting;
	while (state != s_exit) {
		switch (state) {
			case s_waiting:
				a_display[0] = 'S';
				a_display[1] = '0' + p_tag->step;
				a_display[2] = '(';
				a_display[3] = '0' + ((p_tag->groups / 10) % 10);
				a_display[4] = '0' + (p_tag->groups % 10);
				a_display[5] = '/';
				a_display[6] = '1';
				a_display[7] = '2';
				a_display[8] = ')';
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,70,200,16,16, str_46_tag,6);
				lcd_print_hex(30, 40,200,24,24, p_tag->a_id,4);
				LCD_ShowString(30,70,200,16,16, a_display,9);
				delay_us(T_DISPLAY*1000);

				if (p_tag->groups == 0)
					state = s_exit;
				else
					state = s_show_crack;
				break;
			case s_show_crack:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,70,200,16,16, str_are_crack, 10);
				LCD_ShowString(30,70,200,16,16, str_next_r, 6);
				LCD_ShowString(30,70,200,16,16, str_crack_c, 7);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_show_ori_copy;
				else
					state = s_crack;
				break;
			case s_show_ori_copy:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,70,200,16,16, str_copy_sniff_data, 14);
				LCD_ShowString(30,70,200,16,16, str_exit_r, 6);
				LCD_ShowString(30,70,200,16,16, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else
					state = s_ori_copy;
				break;
			case s_ori_copy:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,70,200,16,16, str_insert, 6);
				LCD_ShowString(30,70,200,16,16, str_new_46, 6);
				LCD_ShowString(30,70,200,16,16, str_exit_r, 6);
				LCD_ShowString(30,70,200,16,16, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else {
					lcd_print_blank(0);
					LCD_ShowString(30,70,200,16,16, str_copying, 7);
					if (ts06_copy_ts06(p_tag))
						LCD_ShowString(30,70,200,16,16, str_copy_ok, 8);
					else
						LCD_ShowString(30,70,200,16,16, str_copy_ng, 8);
					delay_us(T_DISPLAY);
				}
				break;
			case s_crack:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,40,200,16,16, str_insert_ori_key, 14);
				LCD_ShowString(30,60,200,16,16, str_exit_r, 6);
				LCD_ShowString(30,80,200,16,16, str_crack_c, 7);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else {
					copy_bytes(sk_uid, p_tag->a_id, 4);
					a_display[0] = '/';
					a_display[1] = ' ';
					a_display[2] = '0';
					a_display[3] = '1';
					a_display[4] = '2';
					for (i_group = 0; i_group < 24; i_group += 2) {
						LCD_Clear(BACK_COLOR);
						LCD_ShowString(30,40,200,16,16, str_reading, 7);
						//lcd_print_bcd(0, 1, i_group/2+1);
						LCD_ShowString(30,60,200,16,16, a_display, 5);
						if (Fccrack46(p_tag->tmcf_dcs, p_tag->a_sign[i_group], p_tag->a_keys))
							break;
					}
					pcf7936_tag1.tmcf_dcs = p_tag->tmcf_dcs;
					pcf7936_tag1.tmcf_enc = e_pcf7936_cipher;
					copy_bytes(pcf7936_tag1.secret_key, p_tag->a_keys, 6);
					LCD_Clear(BACK_COLOR);
					if (i_group >= 24) {
						state = s_exit;
					}
					else if (!pcf7936_read_tag(&pcf7936_tag1, 3, 7)) {
						state = s_exit;
					}
					else {
						copy_bytes(&(pcf7936_tag1.a_page[2][2]), pcf7936_tag1.secret_key, 2);
						copy_bytes(pcf7936_tag1.a_page[1], &(pcf7936_tag1.secret_key[2]), 4);
						LCD_ShowString(30,70,200,16,16, str_crack_ok, 8);
						delay_us(T_DISPLAY);
						state = s_show_skey;
					}

					if (state == s_exit) {
						LCD_ShowString(30,70,200,16,16, str_crack_no, 16);
						LCD_ShowString(30,70,200,16,16, &str_crack_no[16], 16);
						while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
					}
				}
				break;

			case s_show_skey:
				LCD_Clear(BACK_COLOR);
				lcd_print_hex(30, 40,200,24,24, pcf7936_tag1.secret_key, 6);
				LCD_ShowString(30,70,200,16,16, str_exit_r, 6);
				LCD_ShowString(30,70,200,16,16, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else
					state = s_crack_copy;
				break;

			case s_crack_copy:
				LCD_Clear(BACK_COLOR);
				LCD_ShowString(30,70,200,16,16, str_insert, 6);
				LCD_ShowString(30,70,200,16,16, str_new_46, 6);
				LCD_ShowString(30,70,200,16,16, str_exit_r, 6);
				LCD_ShowString(30,70,200,16,16, str_copy_c, 6);
				while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
				if (key == KEYPAD_RD_PRES)
					state = s_exit;
				else {
					lcd_print_copying();
					if (tag_copy_pcf7936(&ts06_tag1, &pcf7936_tag_temp, &pcf7936_tag1))
						LCD_ShowString(30,70,200,16,16, str_copy_ok, 8);
					else
						LCD_ShowString(30,70,200,16,16, str_copy_ng, 8);
					LCD_ShowString(30,70,200,16,16, str_exit_r, 6);
					LCD_ShowString(30,70,200,16,16, str_copy_c, 6);
					while ((key = KEY_Scan(0)) == KEYPAD_NO_PRES);
					if (key == KEYPAD_RD_PRES)
						state = s_exit;
					else
						state = s_crack_copy;
				}
				break;

			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return TRUE;
}



