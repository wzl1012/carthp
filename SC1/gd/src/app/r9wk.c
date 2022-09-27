#include "headfile.h"

#if _INIT_TS_TAG
#define R9WK_T_CHARGE			(50)
#else
#define R9WK_T_CHARGE			(80)
#endif
#define R9WK_T_READ				(50)

uint16_t r9wk_calc_crc16(const uint8_t *p_data, uint8_t bytes)
{
	uint8_t i;
	uint16_t crc;

	crc = 0x0000;
	while (bytes--) {
		crc ^= *p_data++;
		for (i = 0; i < 8; ++i) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0x8408;
			}
			else
				crc >>= 1;
		}
	}

	return crc;
}

r9wk_tag_t r9wk_tag_ori;
r9wk_tag_t r9wk_tag_copy;

uint8_t r9wk_find_tag(r9wk_tag_t *p_tag)
{
	uint8_t result;
	uint16_t crc16_calc, crc16_read;

	p_tag->mode = e_r9wk_not;
	result = FALSE;
	if (tms3705_init_charge(0, R9WK_T_CHARGE)) {
		tms3705_driver_off();
		if (!tms3705_rd_data(&(p_tag->start), 12))
			;
		/* remove TMS3705 disturb */
		else if ((p_tag->a_id[8] == 0x00) && (p_tag->a_id[9] == 0x00))
			;
		/* remove TMS3705 disturb */
		else if ((p_tag->start != 0x7E) || (p_tag->stop != 0x7E))
			;
		else {
			crc16_calc = r9wk_calc_crc16(p_tag->a_id, 8);
			crc16_read = ((uint16_t)p_tag->a_id[9] << 8) | (uint16_t)p_tag->a_id[8];
			
			p_tag->mode = (crc16_calc == crc16_read) ? e_r9wk_crc_ok : e_r9wk_crc_ng;
			result =TRUE;
		}
	}
	delay_us(R9WK_T_READ*1000);

	return result;
}

uint8_t r9wk_keypad_interaction(r9wk_tag_t *p_tag)
{
		//uint8_t key;
//	uint8_t a_display[16];
	enum {
 		s_show_tag, s_show_id1, s_show_id2,
		s_pre_copy, s_copying, s_copy_ok, s_copy_ng,
		s_exit
	} state;
	if (p_tag->mode == e_r9wk_not)
		return FALSE;

	state = s_show_tag;
	while (state != s_exit) {
		switch (state) {
			case s_show_tag:
				//lcd_clear_screen();
			  LCD_Fill(10,50,lcddev.width,120,BACK_COLOR);
				LCD_ShowString(10,50,200,16,16, str_4c_tag, 6);
				/*if (p_tag->mode == e_r9wk_crc_ng)
					lcd_print_str(8, 0, str_crc_no, 6);

				lcd_print_str(0, 1, str_read_r, 6);
				lcd_print_str(8, 1, str_copy_c, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)*/
					state = s_show_id1;
			//	else
				//	state = s_pre_copy;
				break;
			case s_show_id1:
				//lcd_print_blank(0);
				lcd_print_hex(10, 70,200,16,16, &(p_tag->start), 6);
				/*while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)*/
					state = s_show_id2;
				//else
				//	state = s_pre_copy;
				break;
			case s_show_id2:
				lcd_print_hex(10,70,200,16,16, &(p_tag->a_id[5]), 6);
				//lcd_print_str(0, 1, str_exit_r, 6);
				//while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				//if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				//else
				//	state = s_pre_copy;
				break;
		/*	case s_pre_copy:
				a_display[0] = 'I';
				a_display[1] = 'n';
				a_display[2] = 's';
				a_display[3] = ' ';
				a_display[4] = 'T';
				a_display[5] = 'P';
				a_display[6] = 'X';
				a_display[7] = '1';
				a_display[8] = '/';
				a_display[9] = 'Y';
				a_display[10] = 'S';
				a_display[11] = '0';
				a_display[12] = '1';
				a_display[13] = ' ';
				lcd_print_str(0, 0, a_display, 14);
				lcd_print_str(0, 1, str_exit_r, 6);
				lcd_print_str(8, 1, str_copy_c, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_copying;
				break;
			case s_copying:
				lcd_print_copying();
				state = s_copy_ok;
				p_tag->end[0] = 0x00;
				p_tag->end[1] = 0x00;*/
				/* !!!!!!!!!!!!!!!! copy r9wk tag !!!!!!!!!!!!!!!! */
			/*	if (ts4c_copy_r9wk(p_tag->a_id))
					;
				else if (tpx1_copy_r9wk(p_tag))
					;
				else if (ys01_copy_r9wk(p_tag))
					;
				else if (ts0c_copy_r9wk(&(p_tag->start)))
					;
				else if (ts0dnew_copy_r9wk(p_tag))
					;
				else
					state = s_copy_ng;

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
					state = s_pre_copy;
				break;*/
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return TRUE;
}

uint8_t hdx_renew_code(hdx_tag_t *p_hdx)
{
	uint8_t a_national[5], a_country[2];
		uint64_t dec64;
	/* 38 bits national code */
	a_national[0] = p_hdx->a_id[0];
	a_national[1] = p_hdx->a_id[1];
	a_national[2] = p_hdx->a_id[2];
	a_national[3] = p_hdx->a_id[3];
	a_national[4] = p_hdx->a_id[4] & 0x3F;
	/* 10 bits country code */
	a_country[0] = (p_hdx->a_id[4] >> 6) | (p_hdx->a_id[5] << 2);
	a_country[1] = (p_hdx->a_id[5] >> 6);

	dec64 = hexs_to_dec_lsbyte(a_national, 5);
	dec64_to_bcd(dec64, p_hdx->code_national, 6);

	dec64 = hexs_to_dec_lsbyte(a_country, 2);
	dec64_to_bcd(dec64, p_hdx->code_country, 2);
	
	return TRUE;
}

/********************************************************
* FSK, 112Bits
********************************************************/
uint8_t hdx_find_tag(hdx_tag_t *p_hdx)
{
	uint8_t result = FALSE;
	p_hdx->is_hdx = FALSE;
	
	if (tms3705_init_charge(0, R9WK_T_CHARGE)) {
		tms3705_driver_off();
		if (!tms3705_rd_data(&(p_hdx->header), 14))
			;
		else if (p_hdx->header != 0x7E)
			;
		else {
			uint16_t crc16_calc = r9wk_calc_crc16(p_hdx->a_id, 8);
			uint16_t crc16_read = (uint16_t)(p_hdx->a_crc[0]);
			crc16_read |= ((uint16_t)(p_hdx->a_crc[1]) << 8);
			if (crc16_calc == crc16_read) {
				result =TRUE;
				p_hdx->is_hdx =TRUE;
				hdx_renew_code(p_hdx);
			}
		}
	}
	delay_us(R9WK_T_READ);

	return result;
}
/*
uint8_t hdx_display_tag(uint8_t num, hdx_tag_t *p_hdx)
{
	uint8_t str_country[3];
	str_country[0] = (p_hdx->code_country[0] & 0x0F) + '0';
	str_country[1] = (p_hdx->code_country[1] >> 4) + '0';
	str_country[2] = (p_hdx->code_country[1] & 0x0F) + '0';
	
	lcd_clear_screen();
	lcd_print_str(0, 0, str_hdx, 3);
	lcd_print_str(0, 1, str_country, 3);
	lcd_print_hex(30, 40,200,24,24, p_hdx->code_national, 6);
	
	return TRUE;
}
*/
#define TPX1_T_CHARGE			(50)
#define TPX1_T_RECHARGE_READ	(2)
#define TPX1_T_RECHARGE_WRITE	(13)

#define TPX1_T_START_WR_L		(745)
#define TPX1_T_START_WR_S		(295)

#define TPX1_PWM_T_ON_H			(590)
#define TPX1_PWM_T_ON_L			(180)
#define TPX1_PWM_T_OFF			(295)

#define TPX1_CMD_GET_PARA		(0x60)
#define TPX1_CMD_GET_PARA_LEN	(4)
#define TPX1_CMD_WR_BEGIN		(0x50)
#define TPX1_CMD_WR_BEGIN_LEN	(4)
#define TPX1_CMD_WR_END			(0x60)
#define TPX1_CMD_WR_END_LEN		(4)
void tpx1_preframe(uint8_t charge_ms, uint16_t off_us)
{
	tms3705_init_charge(0, charge_ms);
	tms3705_driver_off();
	#if 0
	if (off_us == TPX1_T_START_WR_L)
		timer1_delay_us(TPX1_T_START_WR_L - 2);
	else
		timer1_delay_us(TPX1_T_START_WR_S- 2);
	#else
	/* 2015-08-22 */
	timer1_delay_us(off_us);
	#endif
}

void tpx1_pwm_wr_msbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
	uint8_t i, byte=0;

	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;

		tms3705_driver_on();
		if (byte & 0x80)
			delay_us(TPX1_PWM_T_ON_H);
		else
			delay_us(TPX1_PWM_T_ON_L);
		
		tms3705_driver_off();
		delay_us(TPX1_PWM_T_OFF-2);

		byte <<= 1;
	}

	if (recharge_ms > 0) {
		tms3705_driver_on();
		delay_us((uint16_t)recharge_ms);
		tms3705_driver_off();
	}
}

uint8_t tpx1_get_parameter(tpx1_tag_t *p_tag)
{
	uint8_t i, cmd;

	cmd = TPX1_CMD_GET_PARA;
	p_tag->ok_flag = FALSE;
	tpx1_preframe(TPX1_T_CHARGE, TPX1_T_START_WR_L);
	tpx1_pwm_wr_msbit(&cmd, TPX1_CMD_GET_PARA_LEN, TPX1_T_RECHARGE_READ);
	if (!tms3705_rd_data(p_tag->para, 9))
		return FALSE;

	for (i = 0; i < 9; i++) {
		p_tag->para[i] = reverse_byte(p_tag->para[i]);
	}
	
	/* tpx1 write cmd algorithm */
	p_tag->para[4] = (p_tag->para[0]^p_tag->para[1]) | 0x77;
	p_tag->para[5] = (p_tag->para[2]^p_tag->para[3]) | 0x79;
	p_tag->para[6] = (p_tag->para[1]|p_tag->para[2]) ^ 0x97;
	p_tag->para[7] = (p_tag->para[0]|p_tag->para[3]) ^ 0x99;
	p_tag->ok_flag =TRUE;

	return TRUE;
}

uint8_t shift_one_bit_r(uint8_t *p_data, uint8_t bytes, uint8_t bit_in, int8_t step)
{
	uint8_t bit_msb, bit_lsb=0;

	bit_msb = bit_in;
	while (bytes) {
		bit_lsb = (*p_data) & 0x01;
		*p_data >>= 1;
		if (bit_msb)
			*p_data |= 0x80;
		bit_msb = bit_lsb;

		p_data += step;
		bytes--;
	}

	return bit_lsb;
}

/* TPX1 can recopy any times */
uint8_t tpx1_copy_r9wk(const r9wk_tag_t *p_r9wk)
{
	uint8_t i, cmd, result, a_wr[16];
	tpx1_tag_t tpx1;
	
	if (!tpx1_get_parameter(&tpx1))
		return FALSE;

	a_wr[0] = 0x00;
	a_wr[1] = 0x00;
	copy_bytes(&a_wr[2], &(p_r9wk->start), 12);
	a_wr[14] = 0x00;
	a_wr[15] = 0x00;

	/* 5 times */
	for (i = 0; i < 5; i++)
		shift_one_bit_r(&a_wr[15], 16, 0, -1);

	/* 16 bytes */
	for (i = 0; i < 16; i++)
		a_wr[i] = reverse_byte(a_wr[i]);

	/* write to tpx1 4 page */
	for (i = 0; i < 4; i++) {
		tpx1_preframe(TPX1_T_CHARGE, TPX1_T_START_WR_L);
		cmd = TPX1_CMD_WR_BEGIN;
		tpx1_pwm_wr_msbit(&cmd, TPX1_CMD_WR_BEGIN_LEN, 0);
		tpx1_pwm_wr_msbit(&(tpx1.para[4]), 32, 0);
		tpx1_pwm_wr_msbit(&a_wr[i*4], 32, 0);
		cmd = TPX1_CMD_WR_END;
		tpx1_pwm_wr_msbit(&cmd, TPX1_CMD_WR_END_LEN, 0);
		cmd = i<<6;
		tpx1_pwm_wr_msbit(&cmd, 2, TPX1_T_RECHARGE_WRITE);
		delay_us(R9WK_T_READ*1000);
	}

	/* check copy tag */
	r9wk_tag_copy.stop = p_r9wk->stop ^ 0xFF;	// destroy old data, avoid no receive but compare is ok
	r9wk_find_tag(&r9wk_tag_copy);
	result = equal_bytes(&(p_r9wk->start), &(r9wk_tag_copy.start), 12);

	return result;
}

void ts0c_wr_msbit(const uint8_t *p_wr, uint8_t bits, uint8_t recharge_ms)
{
		
	uint8_t i, byte=0;
	tms3705_driver_off();
	delay_us(65*1000);
	tms3705_driver_on();
	delay_us(9*1000);
	tms3705_driver_off();
	delay_us(3*1000);
	for (i = 0; i < bits; i++) {
		if ((i % 8) == 0)
			byte = *p_wr++;

		tms3705_driver_on();
		if (byte & 0x80)
			delay_us(7*1000);
		else
			delay_us(4*1000);

		tms3705_driver_off();
		if (byte & 0x80)
			delay_us(3*1000);
		else
			delay_us(6*1000);
		
		byte <<= 1;
	}

	tms3705_driver_on();
	delay_us((uint16_t)recharge_ms*1000);
	tms3705_driver_off();
}

uint8_t ts0c_wr_ms_rd(uint8_t bytes_wr, const uint8_t *p_wr, uint8_t recharge_ms, uint8_t *p_rd)
{
	uint8_t result = FALSE;
	if (tms3705_init_charge(0x00, 72)) {
		ts0c_wr_msbit(p_wr, bytes_wr*8, recharge_ms);
		/*
		if (tms3705_rd_data(p_rd, 12))
			result =TRUE;
		*/
		result =TRUE;
	}

	delay_us(15*1000);
	return result;
}

uint8_t ts0c_copy_r9wk(const uint8_t *p_wr)
{
	r9wk_tag_t tag;
	uint8_t a_wr[17];
		uint16_t sum = 0;
	uint8_t i;
		uint8_t result = FALSE;
	if (!r9wk_find_tag(&tag))
		return FALSE;
	
	a_wr[0] = 0x56;
	copy_bytes(&a_wr[1], p_wr, 12);
	a_wr[13] = 0x00;
	a_wr[14] = 0x00;
	for ( i = 0; i < 15; i++)
		sum += reverse_byte(a_wr[i]);

	a_wr[15] = reverse_byte((uint8_t)sum);
	a_wr[16] = reverse_byte((uint8_t)(sum>>8));

	if (!ts0c_wr_ms_rd(17, a_wr, 20, &(tag.start)))
		;
	else if (!r9wk_find_tag(&tag))
		;
	else if (!equal_bytes(p_wr, &(tag.start), 12))
		;
	else
		result =TRUE;

	return result;
}

/********************************************************
* Total is 7 bit, and LSBit first
* cmd: 00
* addr: p_xxxx
********************************************************/
uint8_t ts4c_backdoor_read_byte(const uint8_t addr, uint8_t *p_rd)
{
	uint8_t cmd, result;

	if (addr > 15)
		return FALSE;
	cmd = (addr << 3);
	if (calculate_msbit_parity(&addr, 8))
		cmd |= 0x80;

	result = FALSE;
	if (tms3705_init_charge(0, R9WK_T_CHARGE)) {
		tms37145_pwm_wr_lsbit(&cmd, 8, 15);
		if (tms3705_rd_data(p_rd, 12))
			result =TRUE;
	}

	return result;
}

/********************************************************
* Total is 16 bit, and LSBit first
* cmd: 10
* addr: p_xxxx
* byte: p_xxxxxxxx
********************************************************/
uint8_t ts4c_backdoor_write_byte(const uint8_t addr, const uint8_t byte)
{
	uint8_t result, cmd[3], a_read[12];

	if (addr > 15)
		return FALSE;

	cmd[0] = ((addr<<3) | 0x05);
	if (calculate_msbit_parity(&addr, 8))
		cmd[0] |= 0x80;
	
	cmd[1] = byte;
	if (calculate_msbit_parity(&byte, 8))
		cmd[2] = 0x01;
	else
		cmd[2] = 0x00;

	result = FALSE;
	if (tms3705_init_charge(0, R9WK_T_CHARGE)) {
		tms37145_pwm_wr_lsbit(&cmd[0], 17, 15);
		if (tms3705_rd_data(a_read, 12))
			if (a_read[1] == byte) {
				result =TRUE;
			}
	}

	return result;
}

uint8_t ts4c_copy_r9wk(const uint8_t *p_wr)
{
	uint8_t i;
	for (i = 0; i < 10; i++) {
		if (!ts4c_backdoor_write_byte(i, p_wr[i]))
			return FALSE;
	}

	return TRUE;
}

uint8_t ts4c_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send = 1;
	p_answer[0] = 0x00;

	switch (p_ask[0]) {
		case 0x01:
			if (ts4c_backdoor_read_byte(p_ask[1], &p_answer[1])) {
				len_send += 12;
				p_answer[0] = p_ask[0];
			}
			break;
			
		case 0x02:
			if (ts4c_backdoor_write_byte(p_ask[1], p_ask[2])) {
				p_answer[0] = p_ask[0];
			}
			break;
			
		case 0x03:
			if (ts4c_copy_r9wk(&p_ask[1])) {
				p_answer[0] = p_ask[0];
			}
			break;

		/* read R9WK tag */
		case 0x05:
			if (tms3705_init_charge(0, p_ask[1])) {
				tms3705_driver_off();
				if (tms3705_rd_data(&p_answer[1], 12)) {
					len_send += 12;
					p_answer[0] = p_ask[0];
				}
			}
			break;
			
		default:
			break;
	}
	
	return len_send;
}
/*
void tag4c_fast_read_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "4CFastRead V1.1", 15);
	//led_on();//
	//delay_us(500);
	//led_off();
}
*/
uint8_t tag4c_fast_read(void)
{
	uint8_t result, i_fail, i_ok, a_rd[13];
	enum {
		s_find_tag, s_ok, s_exit
	} state;

	result = FALSE;
	state = s_find_tag;
	i_ok = 0;
	while (state != s_exit) {
		switch (state) {
			case s_find_tag:
				if (tms3705_init_charge(0, R9WK_T_CHARGE)) {
					tms3705_driver_off();
					if (!tms3705_rd_data(&a_rd[0], 12))
						i_ok = 0;
					else if (equal_byte(&a_rd[1], 0x00, 5))
						i_ok = 0;
					else
						i_ok++;
				}
				
				if (i_ok == 0)
					state = s_exit;
				else if (i_ok >= 2)
					state = s_ok;
				break;
				
			case s_ok:
			//	led_on();
				lcd_print_hex(30, 40,200,24,24, &a_rd[1], 8);

				i_fail = 0;
				while (i_fail < 2) {
					i_fail++;
					if (tms3705_init_charge(0, R9WK_T_CHARGE)) {
						tms3705_driver_off();
						if (tms3705_rd_data(&a_rd[0], 12))
							i_fail = 0;
					}
				}
				//led_off();
				lcd_print_blank(1);
				result =TRUE;
				state = s_exit;
				break;
				
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}
/*
void ts42_init_tag_init(void)
{
	lcd_clear_screen();
	lcd_print_str(0, 0, "TS42Init V1.0", 13);
	//led_on();
	//delay_us(500);
//	led_off();
}
*/
uint8_t ts42_init_tag(void)
{
	static const uint8_t a_tp[16] = {
			0x51, 0xF4, 0x15, 0x05, 0x00, 0x00, 0x00, 0x00, 0x72, 0x37, 
			0x00, 0x00, 0x00, 0x00, 0xFD, 0xF8};
	
	uint8_t result, i, i_fail, a_rd[13];
	enum {
		s_wr_tp15to14, s_wr_tp0to9,
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_wr_tp15to14;
	while (state != s_exit) {
		switch (state) {
			case s_wr_tp15to14:
				i_fail = 0;
				if (!ts4c_backdoor_write_byte(15, a_tp[15]))
					i_fail++;
				if (!ts4c_backdoor_write_byte(14, a_tp[14]))
					i_fail++;

				state = (i_fail > 0) ? s_exit : s_wr_tp0to9;
				break;
				
			case s_wr_tp0to9:
				for (i = 0; i < 10; i++) {
					if (!ts4c_backdoor_write_byte(i, a_tp[i]))
						break;
				}

				state = (i < 10) ? s_exit : s_ok;
				break;

			case s_ok:
				//led_on();
				lcd_print_hex(30, 40,200,24,24, &a_tp[14], 2);

				i_fail = 0;
				while (i_fail < 2) {
					i_fail++;
					if (tms3705_init_charge(0, R9WK_T_CHARGE)) {
						tms3705_driver_off();
						if (tms3705_rd_data(&a_rd[0], 12))
							if (equal_bytes(&a_tp[0], &a_rd[1], 10))
								i_fail = 0;
					}
				}
				//led_off();
				lcd_print_blank(1);
				result =TRUE;
				state = s_exit;
				break;
				
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}




