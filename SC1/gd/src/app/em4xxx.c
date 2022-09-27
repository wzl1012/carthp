#include "headfile.h"

/********************************************************
* em4100 == tag 13
********************************************************/
#define EM4100_MEMORY_BITS			(64)
#define EM4100_HEADER_BITS			(9)

/* global variable, store em4100 mode and user data */
em4100_tag_t em4100_tag1;

/********************************************************
* header: 0_1111_11111(one 0 and nine 1)
*
* mode: code and period
********************************************************/
static uint8_t em4100_synch_header(em4100_mode_t mode)
{
	uint8_t bit_period=0;
		uint8_t result, state, dout_temp;
	uint16_t tcnt1_pre;		// TCNT1 should changed with timer1, tcnt1_pre save previous counter
	const uint16_t quarter = ((uint16_t)bit_period) / 2;		//quarter period for timer1 count
	switch (mode) {
		case e_em4100_mc32:
		case e_em4100_bp32:
			bit_period = 32;
			break;
		case e_em4100_mc64:
		case e_em4100_bp64:
			bit_period = 64;
			break;
		default :
			return FALSE;
	}


	timer1_stop();
	TCNT1 = (65536 - (bit_period * 8 * EM4100_MEMORY_BITS));	//find 4 times
	timer1_clear_ov();
	timer1_start_64();
	result = FALSE;

	/* synch header for manchester code */
	if ((mode == e_em4100_mc32) || (mode == e_em4100_mc64)) {
		state = 0;
		while ((timer1_ov_flag == 0) && (state < 10)) {
			switch (state) {
				case 0:
					if (pcf7991_dout_level)
						state = 1;
					break;
				case 1:
					tcnt1_pre = TCNT_V;
					while (((TCNT1-tcnt1_pre) < (quarter*6)) && (pcf7991_dout_level));
					if ((pcf7991_dout_level==0) && ((TCNT1-tcnt1_pre) > (quarter*3))) {
						tcnt1_pre = TCNT1;
						while (((TCNT1-tcnt1_pre) < (quarter*3)) && (pcf7991_dout_level == 0));
						if ((pcf7991_dout_level) && ((TCNT1-tcnt1_pre) > quarter))
							state = 2;
						else
							state = 0;
					}
					else
						state = 0;
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					tcnt1_pre = TCNT1;
					while (((TCNT1-tcnt1_pre) < (quarter*3)) && (pcf7991_dout_level));
					if ((pcf7991_dout_level==0) && ((TCNT1-tcnt1_pre)>quarter)) {
						tcnt1_pre = TCNT1;
						while (((TCNT1-tcnt1_pre) < (quarter*3)) && (pcf7991_dout_level == 0));
						if ((pcf7991_dout_level) && ((TCNT1-tcnt1_pre)>quarter))
							state++;
						else
							state = 0;
					}
					else
						state = 0;
					break;
				case 9:
					tcnt1_pre = TCNT1;
					while (((TCNT1-tcnt1_pre) < (quarter*3)) && (pcf7991_dout_level));
					if ((pcf7991_dout_level==0) && ((TCNT1-tcnt1_pre)>quarter))
						state = 10;
					else
						state = 0;
					break;
				default :
					state = 0;
					break;
			}
		}
		if (state >= 10) {
			timer1_stop();
			TCNT1 = (/*65536 -*/ (quarter<<1) + 1); 	//reduce 4 us, because cpu running need a little time
			timer1_clear_ov();
			timer1_start_64();
			while (timer1_ov_flag == 0);
			result = TRUE;
		}
	}
	
	/* synch header for bi-phase code */
	else if ((mode == e_em4100_bp32) || (mode == e_em4100_bp64)) {
		state = 1;
		while ((timer1_ov_flag == 0) && (state < 10)) {
			switch (state) {
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					dout_temp = pcf7991_dout_level;
					tcnt1_pre = TCNT1;
					while (((TCNT1-tcnt1_pre) < (quarter*6)) && (dout_temp == pcf7991_dout_level));
					if ((dout_temp != pcf7991_dout_level) && ((TCNT1-tcnt1_pre) > (quarter*3))) {
						state++;
					}
					else
						state = 1;
					break;
				default :
					state = 1;
					break;
			}
		}
		if (state >= 10) {
			result = TRUE;
		}
	}
	
	else
		result = FALSE;
	
	timer1_stop();
	timer1_clear_ov();
	return result;
}

/********************************************************
* function: check 10 line parity bits and check 4 column parity bits
* ......
* last is stop bit and header: 01111 11111
* if parity fail, return FALSE; else return TRUE.
********************************************************/
uint8_t em4100_check_parity(const uint8_t *p_data)
{
	uint8_t byte=0, bit_i;
	uint8_t row, column;
	/* p_data is start with user data, and header is last */
	uint8_t a_data[EM4100_MEMORY_BITS - EM4100_HEADER_BITS];

	/* reorganize user data bits with bytes */
	for (bit_i = 0; bit_i < EM4100_MEMORY_BITS - EM4100_HEADER_BITS; bit_i++) {
		if ((bit_i & 0x07) == 0)
			byte = *p_data++;
		
		if (byte & (0x80 >> (bit_i&0x07)))
			a_data[bit_i] = 1;
		else
			a_data[bit_i] = 0;
	}

	/* check 10 line parity bits */
	for (row = 0; row < 10; row++) {
		if ((a_data[row*5 + 0] + a_data[row*5 + 1] + a_data[row*5 + 2] + a_data[row*5 + 3] + a_data[row*5 + 4]) & 0x01)
			return FALSE;
	}

	/* check 4 column parity bits */
	for (column = 0; column < 4; column++) {
		if ((a_data[0+column] + a_data[5+column] + a_data[10+column] + a_data[15+column] 
		+ a_data[20+column] + a_data[25+column] + a_data[30+column] + a_data[35+column] 
		+ a_data[40+column] + a_data[45+column] + a_data[50+column]) & 0x01)
			return FALSE;
	}

	/* check last bit */
	if (a_data[54])
		return FALSE;

	return TRUE;
}

/********************************************************
* function: find em4100
*
* p_tag: pointer for store user data
* page0: set pcf7991 gain, optimize read performance
********************************************************/
uint8_t em4100_find_tag(em4100_tag_t *p_tag, uint8_t page0)
{
	uint8_t result, a_rd[8];
uint8_t line=0;
	uint8_t col=0;
	pcf7991_active_driver();
	pcf7991_set_page(0, page0);
	pcf7991_read_tag();
	delay_us(50*1000);		//ensure work stable

	p_tag->mode = e_em4100_not;
	result = FALSE;

	/* check manchester and 64 period */
	if (!em4100_synch_header(e_em4100_mc64))
		;
	else if (!pcf7991_decode_mc(64, a_rd, EM4100_MEMORY_BITS, e_dir_msbit))
		;
	else if (em4100_check_parity(a_rd)) {
		result = TRUE;
		p_tag->mode = e_em4100_mc64;
	}

	/* check manchester and 32 period */
	if (result)
		;
	else if (!em4100_synch_header(e_em4100_mc32))
		;
	else if (!pcf7991_decode_mc(32, a_rd, EM4100_MEMORY_BITS, e_dir_msbit))
		;
	else if (em4100_check_parity(a_rd)) {
		result = TRUE;
		p_tag->mode = e_em4100_mc32;
	}
	
	/* check bi-phase and 64 period */
	if (result)
		;
	else if (!em4100_synch_header(e_em4100_bp64))
		;
	else if (!pcf7991_decode_bp(64, a_rd, EM4100_MEMORY_BITS, e_dir_msbit))
		;
	else if (em4100_check_parity(a_rd)) {
		result = TRUE;
		p_tag->mode = e_em4100_bp64;
	}
	
	/* check bi-phase and 32 period */
	if (result)
		;
	else if (!em4100_synch_header(e_em4100_bp32))
		;
	else if (!pcf7991_decode_bp(32, a_rd, EM4100_MEMORY_BITS, e_dir_msbit))
		;
	else if (em4100_check_parity(a_rd)) {
		result = TRUE;
		p_tag->mode = e_em4100_bp32;
	}
	
	pcf7991_inactive_driver();
	delay_us(50*1000);

	if (result) {
		for ( line = 0; line < 5; line++) {
			for ( col = 0; col < 8; col++) {
				if (get_array_bit(e_dir_msbit, a_rd, line*10 + col/4 + col))
					p_tag->a_user[line] |= (0x80 >> col);
				else
					p_tag->a_user[line] &= ~(0x80 >> col);
			}
		}
		
		copy_msbits(&(p_tag->a_data[0]), 0, &a_rd[(EM4100_MEMORY_BITS-EM4100_HEADER_BITS)/8], 
						(EM4100_MEMORY_BITS-EM4100_HEADER_BITS)%8, EM4100_HEADER_BITS);
		copy_msbits(&(p_tag->a_data[EM4100_HEADER_BITS/8]), EM4100_HEADER_BITS%8, &a_rd[0], 
						0, EM4100_MEMORY_BITS-EM4100_HEADER_BITS);
	}

	return result;
}

/********************************************************
* function: check em4100 type and keypad interaction
*
* p_tag: pointer for store user data
********************************************************/
uint8_t em4100_keypad_interaction(em4100_tag_t *p_tag)
{
		//uint8_t key;
		enum {
		s_show_id, s_show_copy_tag, 
		s_copying, s_copy_ok, s_copy_ng,
		s_exit
	} state;
	/* check find tag or not */
	if (p_tag->mode == e_em4100_not)
		return FALSE;

	state = s_show_id;
	while (state != s_exit) {
		switch (state) {
			case s_show_id:
				LCD_ShowString(30,120,200,24,24,"               ",LCD_ROW_LENGTH);
				LCD_ShowString(10,50,200,16,16,"                  ",LCD_ROW_LENGTH);
				//lcd_print_str(0, 0, str_13_tag, 6);
			  LCD_ShowString(10,50,200,16,16, str_13_tag,6);
				/*if (p_tag->mode == e_em4100_mc32)
					lcd_print_str(6, 0, str_mc32, 6);
				else if (p_tag->mode == e_em4100_mc64)
					lcd_print_str(6, 0, str_mc64, 6);
				else if (p_tag->mode == e_em4100_bp32)
					lcd_print_str(6, 0, str_bp32, 6);
				else
					lcd_print_str(6, 0, str_bp64, 6);
				delay_us(T_DISPLAY*1000);*/

				lcd_print_hex(10, 70,200,16,16, p_tag->a_data, 8);
				//lcd_print_str(0, 1, str_exit_r, 6);
				//lcd_print_str(8, 1, str_copy_c, 6);
			//	while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
			//	if (key == KEYPAD_FLAG_READ)
					state = s_exit;
			//	else
				//	state = s_show_copy_tag;
				break;
			/*case s_show_copy_tag:
				lcd_print_insert();
				lcd_print_str(8, 0, str_new_13tag, 8);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_copying;
				break;
			case s_copying:*/
				/* can copy to nova or em4305 */
				/*lcd_print_copying();
				if (nova_copy_em4100(&nova_tag_copy, p_tag))
					state = s_copy_ok;
				else if (em4305_copy_em4100(p_tag))
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
				break;*/
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return TRUE;
}

uint8_t em4102_display_tag(uint8_t num, const em4100_tag_t *p_tag)
{
	LCD_ShowString(30,120,200,24,24,"               ",LCD_ROW_LENGTH);
	LCD_ShowString(10,50,200,16,16,"                  ",LCD_ROW_LENGTH);
	//lcd_print_str(0, 0, str_em4102, 6);
	LCD_ShowString(10,50,200,16,16,str_em4102,6);
	lcd_print_hex(10,70,200,16,16, p_tag->a_user, 5);
	
	return TRUE;
}

#define EM4305_CMD_LOGIN		(0x0C << 1)
#define EM4305_CMD_WRITE		(0x0A << 1)
#define EM4305_CMD_READ			(0x09 << 1)
#define EM4305_CMD_PROTECT		(0x03 << 1)
#define EM4305_CMD_DISABLE		(0x05 << 1)

#define EM4305_T_FF_STOP		(32 * 8)	// first field stop time
#define PCF7991_PAGE0_EM4305	(0x0B)		// actual debug, 0x0B better than 0x0F

/********************************************************
* function: array data convert to actual user data
*
* p_array: pointer to array data
* p_word: pointer to actual user data
*
* return: if parity correct, return TRUE
********************************************************/
uint8_t em4305_array_to_word(const uint8_t * const p_array, uint8_t *p_word)
{
	uint64_t temp;
	uint8_t i, parity;

	/* array connect to integer */
	i = 6;
	while (i > 0) {
		i--;
		temp <<= 8;
		temp |= p_array[i];
	}

	/* check 4 line parity bits */
	for (i = 0; i < 4; i++) {
		/* get one actual byte */
		p_word[i] = (uint8_t)temp;
		temp >>= 8;
		/* get parity bit */
		parity = (uint8_t)temp & 0x01;
		temp >>= 1;

		if (parity != calculate_msbit_parity(&p_word[i], 8))
			return FALSE;
	}
	
	/* check 8 column parity bits */
	parity = (uint8_t)temp;
	for (i = 0; i < 4; i++) {
		parity ^= p_word[i];
	}
	if (parity > 0)
		return FALSE;

	return TRUE;
}

/********************************************************
* function: actual user data convert to array data
*
* p_word: pointer to actual user data
* p_array: pointer to array data
********************************************************/
void em4305_word_to_array(const uint8_t * const p_word, uint8_t *p_array)
{
	uint64_t temp;
	uint8_t i, parity;

	/* calculate 8 column parity bits */
	parity = 0;
	for (i = 0; i < 4; i++) {
		parity ^= p_word[i];
	}
	temp = parity;

	/* set 4 line array data */
	i = 4;
	while (i > 0) {
		i--;
		temp <<= 1;
		if (calculate_msbit_parity(&p_word[i], 8))
			temp |= 1;
		temp <<= 8;
		temp |= p_word[i];
	}

	/* integer separate for array */
	for (i = 0; i < 6; i++) {
		p_array[i] = (uint8_t)temp;
		temp >>= 8;
	}
}

void em4305_control_power(uint8_t flag)
{
	if (flag) {
		pcf7991_active_driver();
		pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
		pcf7991_read_tag();
		delay_us(60*1000);
	}
	else {
		pcf7991_inactive_driver();
		delay_us(40*1000);
	}
}

static void em4305_wr_lsbits(uint8_t first_stop, const uint8_t *p_wr, const uint8_t bits)
{
		uint8_t i, byte=0;
	if (first_stop) {
		pcf7991_driver_off();
		delay_us(EM4305_T_FF_STOP);
	}
	
	
	for (i = 0; i < bits; i++) {
		pcf7991_driver_on();
		if ((i % 8) == 0)
			byte = *p_wr++;
		
		timer1_stop();
		TCNT1 = 65535-(2 * 2);			// see assembler code
		timer1_start_8();
		while (TCNT_V < (19*8 * 2));
		
		if ((byte & 0x01) == 0)
			pcf7991_driver_off();
		while (TCNT_V < (254 * 2));
		
		byte >>= 1;
	}

	pcf7991_driver_on();		
	timer1_stop();
}

uint8_t em4305_check_preamble(modulation_t modu, uint8_t period)
{
		uint8_t read = 0;
	/* bp find 8 preamble bit, mc find 7.5 preamble bit */
	if (modu == e_modu_bp) {
		/* first 4 bit unreliable, miss */
		switch (period) {
			case 8:
				delay_us(8*8 * 4);
				break;
			case 16:
				delay_us(16*8 * 4);
				break;
			case 32:
				delay_us(32*8 * 4);
				break;
			case 40:
				delay_us(40*8 * 4);
				break;
			case 64:
				delay_us(64*8 * 4);
				break;
			default:
				return FALSE;
		}
	}
	else {
		/* first 3.5 bit unreliable, miss */
		switch (period) {
			case 8:
				delay_us(8*7 * 4);
				break;
			case 16:
				delay_us(16*7 * 4);
				break;
			case 32:
				delay_us(32*7 * 4);
				break;
			case 40:
				delay_us(40*7 * 4);
				break;
			case 64:
				delay_us(64*7 * 4);
				break;
			default:
				return FALSE;
		}
	}

//	led_flicker();

	if (modu == e_modu_bp)
		pcf7991_decode_bp(period, &read, 4, e_dir_lsbit);
	else
		pcf7991_decode_mc(period, &read, 4, e_dir_lsbit);

//	led_flicker();
	return (read == 0x05) ? TRUE : FALSE;
}

/* 32 bit data is sent using the command data structure format */
uint8_t em4305_read_word(modulation_t modu, uint8_t page, uint8_t *p_rd)
{
		const uint8_t cmd = EM4305_CMD_READ;
	uint8_t result, a_read[6];
	uint8_t (* pf_decode)(uint8_t , uint8_t *, uint8_t , uint8_t );
	if (page > 15)
		return FALSE;
	

	if (calculate_msbit_parity(&page, 8))
		page |= 0x40;

	pf_decode = (modu == e_modu_bp) ? pcf7991_decode_bp : pcf7991_decode_mc;

	result = FALSE;
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &page, 7);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();
	delay_us(720);
	if (!em4305_check_preamble(modu, 32))
		;
	else if (!pf_decode(32, &a_read[0], 45, e_dir_lsbit))
		;
	else if (em4305_array_to_word(&a_read[0], p_rd)) {
		result = TRUE;
	}

	delay_us(50*1000);
	return result;
}

uint8_t em4305_write_word(uint8_t page, const uint8_t *p_wr)
{
		const uint8_t cmd = EM4305_CMD_WRITE;
	uint8_t result, a_wr[6];
	if (page > 15)
		return FALSE;

	em4305_word_to_array(p_wr, a_wr);
	if (calculate_msbit_parity(&page, 8))
		page |= 0x40;
	
	result = FALSE;
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &page, 7);
	em4305_wr_lsbits(FALSE, &a_wr[0], 45);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();

	delay_us(100*1000);
	return result;
}

uint8_t em4305_login_protect(uint8_t cmd, const uint8_t *p_wr)
{
		uint8_t result, a_wr[6];
	switch (cmd) {
		case EM4305_CMD_LOGIN:
			break;
		case EM4305_CMD_PROTECT:
			break;
		default :
			return FALSE;
	}
	
	
	result = FALSE;
	em4305_word_to_array(p_wr, a_wr);
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &a_wr[0], 45);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();
	
	delay_us(100*1000);
	return result;
}

uint8_t em4305_disable(void)
{
	uint8_t result, cmd, a_data[4], a_wr[6];
	
	result = FALSE;
	cmd = EM4305_CMD_DISABLE;
	a_data[0] = 0xFF;
	a_data[1] = 0xFF;
	a_data[2] = 0xFF;
	a_data[3] = 0xFF;
	em4305_word_to_array(a_data, a_wr);
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &a_wr[0], 45);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();
	
	delay_us(100*1000);
	return result;
}

/********************************************************
* error status, in order to taisu test
********************************************************/
uint8_t em4305_read_word_e(modulation_t modu, uint8_t page, uint8_t *p_rd)
{
		const uint8_t cmd = EM4305_CMD_READ;
	uint8_t result, a_read[6];
	uint8_t (* pf_decode)(uint8_t , uint8_t *, uint8_t , uint8_t );
	if (page > 15)
		return FALSE;

	if (calculate_msbit_parity(&page, 8))
		page |= 0x40;
	page ^= 0x40;////////////

	pf_decode = (modu == e_modu_bp) ? pcf7991_decode_bp : pcf7991_decode_mc;

	result = FALSE;
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &page, 7);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();
	delay_us(720);
	if (!em4305_check_preamble(modu, 32))
		;
	else if (!pf_decode(32, &a_read[0], 45, e_dir_lsbit))
		;
	else if (em4305_array_to_word(&a_read[0], p_rd)) {
		result = TRUE;
	}

	delay_us(50*1000);
	return result;
}

/********************************************************
* error status, in order to taisu test
********************************************************/
uint8_t em4305_write_word_e(uint8_t page, const uint8_t *p_wr)
{
		const uint8_t cmd = EM4305_CMD_WRITE;
	uint8_t result, a_wr[6];
	if (page > 15)
		return FALSE;


	em4305_word_to_array(p_wr, a_wr);
	a_wr[0] ^= 0xFF;//////////////
	a_wr[2] ^= 0xFF;//////////////
	if (calculate_msbit_parity(&page, 8))
		page |= 0x40;
	
	result = FALSE;
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &page, 7);
	em4305_wr_lsbits(FALSE, &a_wr[0], 45);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();
	
	delay_us(100*1000);
	return result;
}

/********************************************************
* error status, in order to taisu test
********************************************************/
uint8_t em4305_login_protect_e(uint8_t cmd, const uint8_t *p_wr)
{
		uint8_t result, a_wr[6];
	switch (cmd) {
		case EM4305_CMD_LOGIN:
			break;
		case EM4305_CMD_PROTECT:
			break;
		default :
			return FALSE;
	}
	
	
	result = FALSE;
	em4305_word_to_array(p_wr, a_wr);
	a_wr[0] ^= 0xFF;//////////////
	a_wr[2] ^= 0xFF;//////////////
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &a_wr[0], 45);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();
	
	delay_us(100*1000);
	return result;
}

/********************************************************
* error status, in order to taisu test
********************************************************/
uint8_t em4305_disable_e(void)
{
	uint8_t result, cmd, a_data[4], a_wr[6];
	
	result = FALSE;
	cmd = EM4305_CMD_DISABLE;
	a_data[0] = 0xFF;
	a_data[1] = 0xFF;
	a_data[2] = 0xFF;
	a_data[3] = 0xFF;
	em4305_word_to_array(a_data, a_wr);
	a_wr[0] ^= 0xFF;//////////////
	a_wr[2] ^= 0xFF;//////////////
	pcf7991_write_tag_n(0);
	em4305_wr_lsbits(TRUE, &cmd, 5);
	em4305_wr_lsbits(FALSE, &a_wr[0], 45);
	pcf7991_set_page(0, PCF7991_PAGE0_EM4305);
	pcf7991_read_tag();
	
	delay_us(100*1000);
	return result;
}

uint8_t em4305_copy_em4100(const em4100_tag_t *p_em4100)
{
	uint8_t i, result;
	uint8_t a_cfg[4] = {0x00, 0x00, 0x00, 0x00};
	em4100_tag_t tag_wr;

	/* data rate */
	if ((p_em4100->mode == e_em4100_mc32) || (p_em4100->mode == e_em4100_bp32))
		a_cfg[0] |= 0x0F;
	else if ((p_em4100->mode == e_em4100_mc64) || (p_em4100->mode == e_em4100_bp64))
		a_cfg[0] |= 0x1F;
	else
		return FALSE;
	/* modulation */ 
	if ((p_em4100->mode == e_em4100_mc32) || (p_em4100->mode == e_em4100_mc64))
		a_cfg[0] |= 0x40;
	else if ((p_em4100->mode == e_em4100_bp32) || (p_em4100->mode == e_em4100_bp64))
		a_cfg[0] |= 0x80;
	else
		return FALSE;
	/* memory size, 64bits */
	a_cfg[1] |= 0x80;
	a_cfg[2] |= 0x01;

	result = FALSE;
	/* em4100 is MSBit, em4305 is LSBit */
	for (i = 0; i < 8; i++) {
		tag_wr.a_data[i] = reverse_byte(p_em4100->a_data[i]);
	}
	em4305_control_power(TRUE);
	em4305_write_word(4, a_cfg);
	em4305_write_word(5, &(tag_wr.a_data[0]));
	em4305_write_word(6, &(tag_wr.a_data[4]));
	em4305_control_power(FALSE);

	em4100_find_tag(&tag_wr, PCF7991_PAGE0_EM4100);
	result = equal_bytes(tag_wr.a_data, p_em4100->a_data, 8);

	return result;
}

uint8_t em4305_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send;

	len_send = 1;
	p_answer[0] = 0x00;
	if (p_ask[0] == 0x01) {
		if (em4305_read_word((modulation_t)p_ask[1], p_ask[2], &p_answer[1])) {
			p_answer[0] = p_ask[0];
			len_send += 4;
		}
	}
	else if (p_ask[0] == 0x02) {
		em4305_write_word(p_ask[1], &p_ask[2]);
	}
	else if (p_ask[0] == 0x03) {
		em4305_login_protect(EM4305_CMD_LOGIN, &p_ask[1]);
	}
	else if (p_ask[0] == 0x04) {
		em4305_login_protect(EM4305_CMD_PROTECT, &p_ask[1]);
	}
	else if (p_ask[0] == 0x05) {
		em4305_disable();
	}

	/********************************************************
	* error status, in order to taisu test
	********************************************************/
#if 0
	else if (p_ask[0] == 0x21) {
		if (em4305_read_word_e((modulation_t)p_ask[1], p_ask[2], &p_answer[1])) {
			p_answer[0] = p_ask[0];
			len_send += 4;
		}
	}
	else if (p_ask[0] == 0x22) {
		em4305_write_word_e(p_ask[1], &p_ask[2]);
	}
	else if (p_ask[0] == 0x23) {
		em4305_login_protect_e(EM4305_CMD_LOGIN, &p_ask[1]);
	}
	else if (p_ask[0] == 0x24) {
		em4305_login_protect_e(EM4305_CMD_PROTECT, &p_ask[1]);
	}
	else if (p_ask[0] == 0x25) {
		em4305_disable_e();
	}
#endif

	else if (p_ask[0] == 0x11) {
		em4305_control_power(p_ask[1]);
	}
	
	return len_send;
}




/********************************************************
* em4170 == tag 48
********************************************************/
#define EM4170_T_WA						(128)
#define EM4170_T_WALB					(672)
#define EM4170_T_WEE					(3072)
#define EM4170_T_CHARGE					(70)	// modify_V220 (50) 
#define EM4170_DELAY_WRITE_WORD			(25)	// ms
#define EM4170_DELAY_SEND_PIN			(30)	// ms
#define EM4170_DELAY_ANTHEN				(130)	// us
#define EM4170_DELAY_READ				(130)	// us, read em4170 delay time after command

em4170_tag_t em4170_tag1;

/********************************************************
d15 d14 d13 d12  p3
d11 d10 d09 d08  p2
d07 d06 d05 d04  p1
d03 d02 d01 d00  p0
pc3 pc2 pc2 pc1  0
********************************************************/
void em4170_word_to_array(uint16_t word, uint8_t *p_array)
{
	uint8_t temp, parity;

	temp = ((word & 0xF000) >> 8);
	parity = calculate_msbit_parity(&temp, 4);
	p_array[0] = (temp | (parity << 3));

	temp = ((word & 0x0F00) >> 4);
	parity = calculate_msbit_parity(&temp, 4);
	p_array[1] = (temp | (parity << 3));

	temp = (word & 0x00F0);
	parity = calculate_msbit_parity(&temp, 4);
	p_array[2] = (temp | (parity << 3));

	temp = ((word & 0x000F) << 4);
	parity = calculate_msbit_parity(&temp, 4);
	p_array[3] = (temp | (parity << 3));

	temp = ((p_array[0]&0x80) ^ (p_array[1]&0x80) ^ (p_array[2]&0x80) ^ (p_array[3]&0x80));
	temp |= ((p_array[0]&0x40) ^ (p_array[1]&0x40) ^ (p_array[2]&0x40) ^ (p_array[3]&0x40));
	temp |= ((p_array[0]&0x20) ^ (p_array[1]&0x20) ^ (p_array[2]&0x20) ^ (p_array[3]&0x20));
	temp |= ((p_array[0]&0x10) ^ (p_array[1]&0x10) ^ (p_array[2]&0x10) ^ (p_array[3]&0x10));
	p_array[4] = temp;
}

/********************************************************
* return: if find liw, return TRUE; else return FALSE
********************************************************/
static uint8_t em4170_find_liw(void)
{
	uint8_t result;
	uint16_t tcnt_pre=0;
	enum {
		miss_high, miss_low,
		high_256us, low_384us, high_128us, low_512us,
		find_liw
	} state;
	
	timer1_stop();
	timer1_clear_ov();
	TCNT1 =0xFFFF;			// find time 32768 us
	timer1_start_8();

	state = miss_high;
	result = FALSE;
	while ((!result) && (timer1_ov_flag == 0)) {
		switch (state) {
			case miss_high:
				if (pcf7991_dout_level == 0)
					state = miss_low;
				break;
			case miss_low:
				if (pcf7991_dout_level) {
					tcnt_pre = TCNT_V;
					state = high_256us;
				}
				break;
			case high_256us:
				if (pcf7991_dout_level == 0) {
					if (((TCNT_V-tcnt_pre)>=(206*2)) && ((TCNT_V-tcnt_pre)<=(306*2))){
						tcnt_pre = TCNT_V;
						state = low_384us;
					}
					else
						state = miss_low;
				}
				break;
			case low_384us:
				if (pcf7991_dout_level) {
					if (((TCNT_V-tcnt_pre)>=(334*2)) && ((TCNT_V-tcnt_pre)<=(434*2))){
						tcnt_pre = TCNT_V;
						state = high_128us;
					}
					else {
						tcnt_pre = TCNT_V;
						state = high_256us;
					}
				}
				break;
			case high_128us:
				if (pcf7991_dout_level == 0) {
					if (((TCNT_V-tcnt_pre)>=(96*2)) && ((TCNT_V-tcnt_pre)<=(160*2))){
						tcnt_pre = TCNT_V;
						state = low_512us;
					}
					else
						state = miss_low;
				}
				break;
			case low_512us:
				if (pcf7991_dout_level) {
					if (((TCNT_V-tcnt_pre)>=(462*2)) && ((TCNT_V-tcnt_pre)<=(562*2)))
						state = find_liw;
					else {
						tcnt_pre = TCNT_V;
						state = high_256us;
					}
				}
				break;
			case find_liw:
				result = TRUE;
				break;
			default :
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	return result;
}

/********************************************************
* return: if find ack, return TRUE; else return FALSE
********************************************************/
static uint8_t em4170_find_ack(void)
{
	uint8_t result;
	uint16_t tcnt_pre=0;
	enum {
		miss_high,
		miss_low1, miss_high2, low3_384us, high4_128us, low5_384us, high6_128us,
		find_ack, s_error
	} state;
	
	timer1_stop();
	timer1_clear_ov();
	TCNT1 =0xFFFF;				// max time is 32.768 ms
	timer1_start_8();

	state = miss_high;
	result = FALSE;
	while ((result == FALSE) && (timer1_ov_flag == 0)) {
		switch (state) {
			case miss_high:
				if (pcf7991_dout_level == 0) {
					state = miss_low1;
				}
				break;
			case miss_low1:
				if (pcf7991_dout_level) {
					tcnt_pre = TCNT_V;
					state = miss_high2;
				}
				break;
			case miss_high2:
				if (pcf7991_dout_level == 0) {
					tcnt_pre = TCNT_V;
					state = low3_384us;
				}
				break;
			case low3_384us:
				if (pcf7991_dout_level) {
					if (((TCNT_V-tcnt_pre)>=(324*2)) && ((TCNT_V-tcnt_pre)<=(444*2))) {
						tcnt_pre = TCNT_V;
						state = high4_128us;
					}
					else
						state = s_error;
				}
				break;
			case high4_128us:
				if (pcf7991_dout_level == 0) {
					if (((TCNT_V-tcnt_pre)>=(68*2)) && ((TCNT_V-tcnt_pre)<=(188*2))) {
						tcnt_pre = TCNT_V;
						state = low5_384us;
					}
					else
						state = s_error;
				}
				break;
			case low5_384us:
				if (pcf7991_dout_level) {
					if (((TCNT_V-tcnt_pre)>=(324*2)) && ((TCNT_V-tcnt_pre)<=(444*2))) {
						tcnt_pre = TCNT_V;
						state = high6_128us;
					}
					else
						state = s_error;
				}
				break;
			case high6_128us:
				state = find_ack;
				break;
			case find_ack:
				result = TRUE;
				break;
			case s_error:
				timer1_stop();
				timer1_clear_ov();
				return FALSE;
			default :
				break;
		}
	}

	timer1_stop();
	timer1_clear_ov();

	return result;
}

/********************************************************
* p_wr: pointer for write data
* bits: bits of write
********************************************************/
static void em4170_wr_msbits(const uint8_t *p_wr, const uint8_t bits)
{
	uint8_t i, byte=0;
	
	for (i = 0; i < bits; i++) {
		pcf7991_driver_on();
		if ((i % 8) == 0)
			byte = *p_wr++;
		
		timer1_stop();
		TCNT1 = 65535-(2 * 2);	//adjust time, see assembler code
		timer1_start_8();

		while (TCNT_V < (20 * 2));

		/* MSBit first */
		if ((byte & 0x80) == 0)
			pcf7991_driver_off();		
		while (TCNT_V < (128 * 2));
		
		pcf7991_driver_on();		
		while (TCNT_V < (274 * 2));

		byte <<= 1;
	}

	timer1_stop();
}

uint8_t em4170_id_um_mode(uint8_t cmd, uint8_t *p_rd)
{
	uint8_t bytes, result, a_read[11];

	/* check command */
	switch (cmd) {
		case EM4170_CMD_ID_MODE:
		case EM4170_CMD_UM_MODE_1:
			bytes = 4;
			break;
		case EM4170_CMD_UM_MODE_2:
			bytes = 8;
			break;
		default:
			return FALSE;
	}
	cmd >>= 2;	//the RM pattern consists of 2 bits "0" send by the transceiver

	result = FALSE;
	if (em4170_find_liw()) {
		pcf7991_write_tag_n(0);
		delay_us(400);
		em4170_wr_msbits(&cmd, 6);
		//pcf7991_tagrd_gsetproc();
		pcf7991_read_tag();
		delay_us(EM4170_DELAY_READ);
		if (!pcf7991_decode_miss_low())
			;
		else if (!pcf7991_decode_mc(32, a_read, (bytes*8 + 16), e_dir_msbit))
			;
		/* check 16 bits header */
		else if ((a_read[0] == 0xFF) && (a_read[1] == 0xF0)) {
			copy_bytes(p_rd, &a_read[2], bytes);
			result = TRUE;
		}
	}
	
	if (result)
		delay_us(1*1000);
	return result;
}

/********************************************************
* p_rd length >= 32
********************************************************/
uint8_t em4170_read_all(uint8_t *p_rd)
{
	/* caution, these code have security infomation, can't compiler */
#if 0
	uint8_t cmd, result;
	uint8_t a_read[34];
	
	cmd = EM4170_CMD_READ_ALL >> 2;
	result = FALSE;
	if (em4170_find_liw()) {
		pcf7991_write_tag_n(0);
		em4170_wr_msbits(&cmd, 6);
		pcf7991_read_tag();

		delay_us(EM4170_DELAY_READ);
		if (!pcf7991_decode_miss_low())
			;
		else if (!pcf7991_decode_mc(32, a_read, 16+128, e_dir_msbit))
			;
		else if (!pcf7991_decode_mc(32, &a_read[2+16], 128, e_dir_msbit))
			;
		else if ((a_read[0] == 0xFF) && (a_read[1] == 0xF0)) {
			copy_bytes(p_rd, &a_read[2], 32);
			result = TRUE;
		}
	}

	return result;
#else
	return FALSE;
#endif
}

uint8_t em4170_send_pin(const uint8_t *p_id, const uint8_t *p_pin)
{
	const uint8_t cmd = EM4170_CMD_SEND_PIN >> 2;
	uint8_t result;
	uint8_t a_read[6];

	result = FALSE;
	if (em4170_find_liw()) {
		pcf7991_write_tag_n(0);
		em4170_wr_msbits(&cmd, 6);
		em4170_wr_msbits(p_id, 32);
		em4170_wr_msbits(p_pin, 32);
		pcf7991_read_tag();
		delay_us(EM4170_DELAY_SEND_PIN*1000);
		
		if (pcf7991_decode_miss_low())
			if (pcf7991_decode_mc(32, a_read, 48, e_dir_msbit))
				if ((a_read[0] == 0xFF) && (a_read[1] == 0xF0) 
				&& (a_read[2] == p_id[0]) && (a_read[3] == p_id[1]) 
				&& (a_read[4] == p_id[2]) && (a_read[5] == p_id[3])) {
					result = TRUE;
				}
	}
	
	if (!result)
		delay_us(256 * 48 / 1000*1000);
	return result;
}

/* send 56bits rn, send 28 bits frn, and receive 20bits grn */
uint8_t em4170_auth(const uint8_t *p_rn, const uint8_t *p_frn, uint8_t *p_grn)
{
	uint8_t result, cmd, temp;
	uint8_t a_read[5];

	cmd = EM4170_CMD_AUTHEN>> 2;
	temp = 0;
	result = FALSE;
	if (em4170_find_liw()) {
		pcf7991_write_tag_n(0);
		em4170_wr_msbits(&cmd, 6);
		em4170_wr_msbits(p_rn, 56);
		em4170_wr_msbits(&temp, 7);
		em4170_wr_msbits(p_frn, 28);
		pcf7991_read_tag();
		pcf7991_tagrd_gsetproc();
		//delay_us(EM4170_DELAY_ANTHEN);

		if (!pcf7991_decode_miss_low())
			;
		else if (!pcf7991_decode_mc(32, a_read, 36, e_dir_msbit))
			;
		else if ((a_read[0]==0xFF) && (a_read[1]==0xF0)) {
			a_read[4] &= 0xF0;
			copy_bytes(p_grn, &a_read[2], 3);
			result = TRUE;
		}
	}

	if (result)
		delay_us(1*1000);
	return result;
}

/********************************************************
* Write-Mode can only be entered if LB0 = "0"(LB1 = "X")
********************************************************/
uint8_t em4170_write_word(uint8_t addr, uint16_t word)
{
	const uint8_t cmd = EM4170_CMD_WR_WORD >> 2;
	uint8_t i, result;
	uint8_t a_wr_data[5];
uint8_t trys;
	addr <<= 4;
	if (calculate_msbit_parity(&addr, 4) == 0x01)
		addr |= 0x08;
	em4170_word_to_array(word, a_wr_data);
	
	result = FALSE;
	for (trys = 0; (trys<2) && (!result); trys++) {
		if (em4170_find_liw()) {
			pcf7991_write_tag_n(0);
			em4170_wr_msbits(&cmd, 6);
			em4170_wr_msbits(&addr, 5);
			for (i = 0; i < 5; i++)
				em4170_wr_msbits(&a_wr_data[i], 5);
			pcf7991_read_tag();
			delay_us(EM4170_DELAY_WRITE_WORD*1000);
			
			result = em4170_find_ack(); 	// check write OK
		}
	}

	if (result)
		delay_us(2*1000);
	return result;
}

void em4170_power_control(uint8_t flag)
{
	if (flag) {
		pcf7991_active_driver();
		delay_us(10);
		pcf7991_set_page(0, PCF7991_PAGE0_EM4170);
		delay_us(10);
		pcf7991_read_tag();
		delay_us(EM4170_T_CHARGE*1000);
	}
	else {
		pcf7991_inactive_driver();
		delay_us(20*1000);
	}
}

uint8_t em4170_copy_em4170(const em4170_tag_t *p_tag)
{
		uint8_t i;
	em4170_power_control(TRUE);

	for (i = 0; i < 3; i++) {
		if (!em4170_write_word(15, ((uint16_t)(p_tag->a_um2[0])<<8) | ((uint16_t)(p_tag->a_um2[1]))))
			;
		else if (!em4170_write_word(14, ((uint16_t)(p_tag->a_um2[2])<<8) | ((uint16_t)(p_tag->a_um2[3]))))
			;
		else if (!em4170_write_word(13, ((uint16_t)(p_tag->a_um2[4])<<8) | ((uint16_t)(p_tag->a_um2[5]))))
			;
		else if (!em4170_write_word(12, ((uint16_t)(p_tag->a_um2[6])<<8) | ((uint16_t)(p_tag->a_um2[7]))))
			;
		else if (!em4170_write_word(9, 0xAAAA))
			;
		else if (!em4170_write_word(8, 0xAAAA))
			;
		else if (!em4170_write_word(7, 0xAAAA))
			;
		else if (!em4170_write_word(6, 0xAAAA))
			;
		else if (!em4170_write_word(5, 0xAAAA))
			;
		else if (!em4170_write_word(4, 0xAAAA))
			;
		else if (!em4170_write_word(3, ((uint16_t)(p_tag->a_id[0])<<8) | ((uint16_t)(p_tag->a_id[1]))))
			;
		else if (!em4170_write_word(2, ((uint16_t)(p_tag->a_id[2])<<8) | ((uint16_t)(p_tag->a_id[3]))))
			;
		else if (!em4170_write_word(0, ((uint16_t)(p_tag->a_um1[2])<<8) | ((uint16_t)(p_tag->a_um1[3]))))
			;
		else if (!em4170_write_word(1, ((uint16_t)(p_tag->a_um1[0])<<8) | ((uint16_t)(p_tag->a_um1[1]))))
			;
		else
			break;
	}

	em4170_power_control(FALSE);

	if (i < 3)
		return TRUE;
	else
		return FALSE;
}

uint8_t em4170_find_tag(em4170_tag_t *p_tag)
{
	/* in order to check crypt key is origin */
	static const uint8_t a_auth_rn[7] = {0x46, 0x91, 0x08, 0x04, 0x06, 0xAB, 0x87};
	static const uint8_t a_auth_frn[4] = {0xB9, 0x6E, 0x71, 0xA0};
	static const uint8_t a_auth_grn[3] = {0x65, 0x23, 0x10};
	
	uint8_t i, result, a_grn[3];
   //pcf7991_confg();
	em4170_power_control(TRUE);
	p_tag->mode = e_no_em4170;
	p_tag->crypt_ori = FALSE;
	result = FALSE;
	/* find 2 times */
	for (i = 0; i < 2; i++) {
		if (!em4170_id_um_mode(EM4170_CMD_ID_MODE, p_tag->a_id))
			continue;
		else if (!em4170_id_um_mode(EM4170_CMD_UM_MODE_1, p_tag->a_um1))
			continue;
		else if (!em4170_id_um_mode(EM4170_CMD_UM_MODE_2, p_tag->a_um2))
			continue;
		else
			break;
	}
	if (i < 2) {
		p_tag->mode = e_yes_em4170;
		result = TRUE;
	}

	for (i = 0; i < 2; i++) {
		if (!em4170_auth(a_auth_rn, a_auth_frn, a_grn))
			continue;
		else if (equal_bytes(a_auth_grn, a_grn, 3)) {
			p_tag->crypt_ori = TRUE;
			break;
		}
	}

	em4170_power_control(FALSE);
	return result;
}

uint8_t em4170_keypad_interaction(em4170_tag_t *p_tag)
{
	
	//uint8_t key;
		//enum {
		//s_waiting, 
		//s_show_copy_tag, s_copying, s_copy_ok, s_copy_ng,
	//	s_exit
	//} state;
	if (p_tag->mode == e_no_em4170)
		return FALSE;

	//LCD_Fill(5,45,lcddev.width,lcddev.height,BACK_COLOR);
	if(p_tag->mode == e_yes_em4170){
	OLED_Clear( );
	//state = s_waiting;
	//while (state != s_exit) {
		//switch (state) {
		//	case s_waiting:
				//LCD_ShowString(30,120,200,24,24,"               ",LCD_ROW_LENGTH);
				//LCD_ShowString(10,50,200,16,16,"                  ",LCD_ROW_LENGTH);
				OLED_ShowString(0, 0, str_48_tag, 16,6);
				//LCD_ShowString(3,33,lcddev.width,12,12,str_48_tag,6);
				//delay_us(T_DISPLAY*1000);			   
				//lcd_print_hex(3,48,lcddev.width,12,12, p_tag->a_id, 4);
				//if(p_tag->ictype==pcf7936_yes)
				//LCD_ShowString(10, 90,200,16,16,"EM4170",7);
			//else
				 // LCD_ShowString(10, 90,200,16,16,"               ",LCD_ROW_LENGTH);
			 if((p_tag->a_um1[3]&0xc0)==0xc0){
				 OLED_ShowString(51, 0, "locked", 16,6);
			   //LCD_ShowString(3,63,lcddev.width,12,12,"locked",6);
			 }else{
				 OLED_ShowString(51, 0,str_locked,16,6);
				 //LCD_ShowString(3,63,lcddev.width,12,12,str_locked,6);
			 }
			 /*
			  LCD_ShowString(3,78,lcddev.width,12,12,"a_um1[0]:",9); 
        lcd_print_hex(48,78,lcddev.width,12,12, &p_tag->a_um1[0],2); 
 				LCD_ShowString(48+4*7, 78,lcddev.width,12,12,"key[0]:",7); 
        lcd_print_hex(48+4*7+7*8, 78,lcddev.width,12,12, &p_tag->key[0],2); 
			
				LCD_ShowString(10, 93,200,16,16,"a_um1[1]:",9); 
        lcd_print_hex(81, 130,200,16,16, &p_tag->a_um1[2],2); 
 				LCD_ShowString(81+4*11, 130,200,16,16,"key[1]:",7); 
        lcd_print_hex(81+4*11+7*8, 130,200,16,16, &p_tag->key[2],2); 
			
				LCD_ShowString(10,108,200,16,16,"a_um2[0]:",9); 
        lcd_print_hex(81,150,200,16,16, &p_tag->a_um2[0],2); 
 				LCD_ShowString(81+4*11,150,200,16,16,"key[2]:",7); 
        lcd_print_hex(81+4*11+7*8,150,200,16,16, &p_tag->key[4],2); 
			
				LCD_ShowString(10, 123,200,16,16,"a_um2[1]:",9); 
        lcd_print_hex(81, 170,200,16,16,&p_tag->a_um2[2],2); 
 				LCD_ShowString(81+4*11, 170,200,16,16,"key[3]:",7); 
        lcd_print_hex(81+4*11+7*8, 170,200,16,16,&p_tag->key[6],2);
			 
			 	LCD_ShowString(10, 138,200,16,16,"a_um2[2]:",9); 
        lcd_print_hex(81, 190,200,16,16,&p_tag->a_um2[4],2); 
 				LCD_ShowString(81+4*11,190,200,16,16,"key[4]:",7); 
        lcd_print_hex(81+4*11+7*8, 190,200,16,16,&p_tag->key[8],2);
				
				LCD_ShowString(10, 150,200,16,16,"a_um2[3]:",9); 
        lcd_print_hex(81, 210,200,16,16,&p_tag->a_um2[6],2); 
 				LCD_ShowString(81+4*11,210,200,16,16,"key[5]:",7); 
        lcd_print_hex(81+4*11+7*8, 210,200,16,16,&p_tag->key[10],2);*/
			 // LCD_ShowString(10,90,200,16,16,"                  ",LCD_ROW_LENGTH);
				//lcd_print_str(0, 1, str_exit_r, 6);
				/*if (p_tag->crypt_ori)
					lcd_print_str(8, 1, str_copy_c, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)*/
				//	state = s_exit;
				/*else if (p_tag->crypt_ori)
					state = s_show_copy_tag;
				else
					state = s_exit;*/
			//	break;
		/*	case s_show_copy_tag:
				lcd_print_insert();
				lcd_print_str(8, 0, str_48_tag, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				else
					state = s_copying;
				break;
			case s_copying:
				lcd_print_copying();
				if (em4170_copy_em4170(p_tag))
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
				break;*/
			//case s_exit:
			//default :
			//	state = s_exit;
				//break;
		//}
	//}
  }
	return TRUE;
}

uint8_t em4170_comm_handle(const uint8_t *p_ask, uint8_t *p_answer)
{
	uint8_t len_send;
	uint8_t a_id[4];

	em4170_power_control(TRUE);

	len_send = 1;
	p_answer[0] = 0x00;
	if (p_ask[0] == 0x01) {
		if (em4170_id_um_mode(EM4170_CMD_UM_MODE_1, &p_answer[1])) {
			len_send = 5;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x02) {
		if (em4170_id_um_mode(EM4170_CMD_ID_MODE, &p_answer[1])) {
			len_send = 5;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x03) {
		if (em4170_id_um_mode(EM4170_CMD_UM_MODE_2, &p_answer[1])) {
			len_send = 9;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x04) {
		if (em4170_read_all(&p_answer[1])) {
			len_send = 33;
			p_answer[0] = p_ask[0];
		}
	}
	else if ((p_ask[0] >= 0x10) && (p_ask[0] <= 0x1F)) {
		if (em4170_write_word(p_ask[0]-0x10, ((uint16_t)(p_ask[1])<<8) | ((uint16_t)p_ask[2]))) {
			len_send = 1;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x21) {
		if (!em4170_id_um_mode(EM4170_CMD_ID_MODE, a_id))
			;
		else if(em4170_send_pin(a_id, &p_ask[1])) {
			len_send = 1;
			p_answer[0] = p_ask[0];
		}
	}
	else if (p_ask[0] == 0x22) {
		if(em4170_auth(&p_ask[1], &p_ask[8], &p_answer[1])) {
			len_send = 4;
			p_answer[0] = p_ask[0];
		}
	}
	
	em4170_power_control(FALSE);
	return len_send;
}

void em4170_fast_read_init(void)
{
	//lcd_clear_screen();
	//lcd_print_str(0, 0, "48FastRead V1.2", 15);
	//led_on();
	//delay_us(500);
	//led_off();

	pcf7991_active_driver();
	pcf7991_set_page(0, PCF7991_PAGE0_EM4170);
	pcf7991_read_tag();
}

uint8_t em4170_fast_read(void)
{
	uint8_t result, a_id[4], a_um1[4], a_uart[16];
	enum {
		s_runing, s_charge,
		s_get_data,
		s_ok, s_exit
	} state;

	result = FALSE;
	state = s_charge;
	while (state != s_exit) {
		switch (state) {
			case s_charge:
				if (em4170_find_liw()) {
					delay_us(5*1000);
					state = s_get_data;
				}
				else
					state = s_exit;
				break;
			case s_get_data:
				if (!em4170_id_um_mode(EM4170_CMD_ID_MODE, a_id))
					state = s_exit;
				else if (!em4170_id_um_mode(EM4170_CMD_UM_MODE_1, a_um1))
					state = s_exit;
				else
					state = s_ok;
				break;
			case s_ok:
			//	led_on();

				a_uart[0] = 0xA5;
				a_uart[1] = 0xFF;
				a_uart[2] = 11;
				a_uart[3] = 0x48;
				a_uart[4] = 0x08;
				a_uart[5] = a_um1[2];
				a_uart[6] = a_um1[3];
				a_uart[7] = a_um1[0];
				a_uart[8] = a_um1[1];
				a_uart[9] = a_id[2];
				a_uart[10] = a_id[3];
				a_uart[11] = a_id[0];
				a_uart[12] = a_id[1];
			//	a_uart[13] = calculate_sum(a_uart, 13);
			//	uart_send_str(a_uart, 14);

				lcd_print_hex(30, 40,200,24,24, &a_uart[5], 8);
				
				while (em4170_find_liw());
				
			//	led_off();
				lcd_print_blank(1);
				state = s_exit;
				result = TRUE;
				break;
			default :
				state = s_exit;
				break;
		}
	}

	return result;
}


#define EM4170_INIT_INTERVAL_SAVE		(0x40)
static uint16_t uid_i_em4170;

void ts48_init_tag_init(void)
{
	uint8_t i, a_addr[2]={0};
	
//	eep_rd_data(EEP_ADDR_EM4170_ID, a_addr, 2);
	uid_i_em4170 = ((uint16_t)a_addr[0]<<8) | ((uint16_t)a_addr[1]);
	uid_i_em4170 += EM4170_INIT_INTERVAL_SAVE;			// jump to next segment

	i = 10;
	if (uid_i_em4170 >= EM4170_ID_MAX_NUM) {
		uid_i_em4170 = 0;
		a_addr[0] = 0;
		a_addr[1] = 0;
	//	for (; i > 0; i--)
		//	if (eep_wr_data(EEP_ADDR_EM4170_ID, a_addr, 2))
			//	break;
	}

	//lcd_clear_screen();
	if (i > 0) {
		//led_on();
		//delay_us(500);
		//led_off();
		debug_printf("\r\nTS48Init V1.7\r\n");
		//lcd_print_str(0, 0, "TS48Init V1.7", 13);
		pcf7991_active_driver();
		pcf7991_set_page(0, PCF7991_PAGE0_EM4170);
		pcf7991_read_tag();
	}
	else {
		//lcd_print_str(0, 0, "ATmega128 error", 15);
		while(1);
	}
}
#if _INIT_TS_TAG
void ts48_init_tag(void)
{
	/* use static: define a_data_ori[] in RAM, because can reduce program ROM */
	static const uint8_t a_data_ori[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	
	uint8_t i, a_id[4], a_um1[4];
	uint8_t a_flash[8], a_uart[20];
	enum {
		s_charge,
		s_send_pin, s_wr_id, s_check_lb1, 
		s_init_ok, s_exit
	} state;

	state = s_charge;
	while (state != s_exit) {
		switch (state) {
			case s_charge:
				if (em4170_find_liw()) {
					delay_us(5*1000);
					state = s_send_pin;
				}
				else
					state = s_exit;
				break;
			case s_send_pin:
				if (!em4170_id_um_mode(EM4170_CMD_ID_MODE, a_id))
					state = s_exit;
				else {
					em4170_send_pin(a_id, a_data_ori);
					state = s_wr_id;
				}
				break;
			case s_wr_id:
				uid_i_em4170++;
				if (uid_i_em4170 >= EM4170_ID_MAX_NUM)
					uid_i_em4170 = 0;
				/* reduce write eeprom times */
				if ((uid_i_em4170 % EM4170_INIT_INTERVAL_SAVE) == 0) {
					a_flash[0] = (uid_i_em4170 >> 8);
					a_flash[1] = uid_i_em4170;
				//	if (!eep_wr_data(EEP_ADDR_EM4170_ID, a_flash, 2))
					//	if (!eep_wr_data(EEP_ADDR_EM4170_ID, a_flash, 2))
						//	eep_wr_data(EEP_ADDR_EM4170_ID, a_flash, 2);
				}

				if (uid_i_em4170 < 4000) {
					for (i = 0; i < 8; i++)
						a_flash[i] = a_em4170_uid1[uid_i_em4170][i];
				}
				else if (uid_i_em4170 < 8000) {
					for (i = 0; i < 8; i++)
						a_flash[i] = a_em4170_uid2[uid_i_em4170-4000][i];
				}
				while ((a_flash[2] & 0xF0) != 0xA0) {
					//lcd_print_str(0, 1, "ID Error", 8);
					debug_printf("\r\n ID Error\r\n");
				//	led_flicker();
				//	delay_us(2000*1000);
				}

				/* write 15-4 word */
				for (i = 15; i >= 4; i--) {
					if (!em4170_write_word(i, 0xAAAA))
						break;
				}

				/* initialization state for s_exit */
				state = s_exit;
				if (i >= 4)
					;
				else if (!em4170_write_word(3, (((uint16_t)a_flash[6]<<8) | (uint16_t)a_flash[7])))
					;
				/* actual find word3 have oxFF 0xFF */
				else if (!em4170_write_word(3, (((uint16_t)a_flash[6]<<8) | (uint16_t)a_flash[7])))
					;
				else if (!em4170_write_word(2, (((uint16_t)a_flash[4]<<8) | (uint16_t)a_flash[5])))
					;
				else if (!em4170_write_word(1, (((uint16_t)a_flash[2]<<8) | (uint16_t)a_flash[3])))
					;
				else if (!em4170_write_word(0, (((uint16_t)a_flash[0]<<8) | (uint16_t)a_flash[1])))
					;
				else
					state = s_check_lb1;
				break;
			case s_check_lb1:
				for (i = 0; i < 3; i++) {
					if (em4170_id_um_mode(EM4170_CMD_UM_MODE_1, a_um1))
						if (a_um1[0] == a_flash[2])
							break;
				}
				if (i < 3)
					state = s_init_ok;
				else
					state = s_exit;
				break;
			case s_init_ok:
			//	led_on();
				
				a_uart[0] = 0xA5;
				a_uart[1] = 0xFF;
				a_uart[2] = 11;
				a_uart[3] = 0x48;
				a_uart[4] = 0x08;
				copy_bytes(&a_uart[5], a_flash, 8);
			//	a_uart[13] = calculate_sum(a_uart, 13);
		//		uart_send_str(a_uart, 14);
				
				lcd_print_hex(30, 40,200,24,24, a_flash, 8);

				while (em4170_find_liw());
				
			//	led_off();
				lcd_print_blank(1);
				state = s_exit;
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
}
#endif

