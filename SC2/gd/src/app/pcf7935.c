#include "headfile.h"

/* 
PCF7935，由基站首先发送一串随机数，应答器再回应经过加密的代码，经过验证后才可启动发动机。
PCF7936，基站不仅发送随机数，同时发送加密信息，通过认证后，应答器才发送加密的应答信号，用于启动。这是目前主要的IMMO应用方式。
PCF7939，采用AES128的加密算法传输数据。
The first CDP codes bits always starts with the lower clipping level (pcf7991 dout level high) */
#define PCF7935_PERIOD_READ					(64)

#define PCF7935_T_RESET_SETUP				(20)
#define PCF7935_T_RESET						(13)

pcf7935_tag_t pcf7935_tag1;

void pcf7935_to_off(void)
{
	pcf7991_inactive_driver();
	delay_1us(PCF7935_T_RESET_SETUP*1000);
}

uint8_t pcf7935_synch(void)
{
	uint8_t result;
	uint16_t tcnt_pre=0;
	enum {
		miss_low,
		high_128us, low_256us, high_1024us, low_1664us,
		find_synch
	} state;
	
	timer1_stop();
	timer1_clear_ov();
	TCNT1 = /*65535 - */55000;			// timeout = 220ms, one cycle = (384*8 + 512*128) = 68608 us
	timer1_start_64();

	result =FALSE;
	state = miss_low;
	while ((result ==FALSE) && (timer1_ov_flag == 0)) {
		switch (state) {
			case miss_low:
				if (pcf7991_dout_level) {
					tcnt_pre = TCNT1;
					state = high_128us;
				}
				break;
			case high_128us:
				if (pcf7991_dout_level == 0) {
					if (((TCNT_V-tcnt_pre)>=(68>>2)) && ((TCNT1-tcnt_pre)<=(188>>2)))
						state = low_256us;
					else
						state = miss_low;
					tcnt_pre = TCNT1;
				}
				break;
			case low_256us:
				if (pcf7991_dout_level) {
					if (((TCNT1-tcnt_pre)>=(192>>2)) && ((TCNT1-tcnt_pre)<=(320>>2)))
						state = high_1024us;
					else
						state = high_128us;
					tcnt_pre = TCNT1;
				}
				break;
			case high_1024us:
				if (pcf7991_dout_level == 0) {
					if (((TCNT1-tcnt_pre)>=(880>>2)) && ((TCNT1-tcnt_pre)<=(1200>>2)))
						state = low_1664us;
					else
						state = miss_low;
					tcnt_pre = TCNT1;
				}
				break;
			case low_1664us:
				if (pcf7991_dout_level) {
					if (((TCNT1-tcnt_pre)>=(1564>>2)) && ((TCNT1-tcnt_pre)<=(1764>>2))){
						state = find_synch;
					}
					else {
						tcnt_pre = TCNT1;
						state = high_128us;
					}
				}
				break;
			case find_synch:
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

uint8_t pcf7935_find_tag(pcf7935_tag_t *p_tag)
{
		uint8_t result =FALSE;
	pcf7935_to_off();
	pcf7991_active_driver();
	delay_1us(10);
	pcf7991_set_page(0, PCF7991_PAGE0_PCF7935);
	delay_1us(10);
	pcf7991_read_tag();
	delay_1us(PCF7935_T_RESET*1000);
	
	p_tag->mode = e_pcf7935_not;
	if (!pcf7935_synch())
		;
	else if (pcf7991_decode_bp(64, p_tag->a_id, 128, e_dir_lsbit)) {
		p_tag->mode = e_pcf7935_yes;
		result = TRUE;
	}

	pcf7935_to_off();
	return result;
}

uint8_t pcf7935_keypad_interaction(pcf7935_tag_t *p_tag)
{
		uint8_t key;
	enum {
		s_waiting, s_show_id1, s_show_id2,
		s_exit
	} state;
	if (p_tag->mode == e_pcf7935_not)
		return FALSE;


	state = s_waiting;
	while (state != s_exit) {
		switch (state) {
			case s_waiting:
				lcd_clear_screen();
				lcd_print_str(0, 0, str_44_tag, 6);
				lcd_print_str(0, 1, str_read_r, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_show_id1;
				break;
			case s_show_id1:
				lcd_clear_screen();
				lcd_print_hex(30, 40,200,24,24, &p_tag->a_id[0], 8);
				lcd_print_str(0, 1, str_next_r, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_show_id2;
				break;
			case s_show_id2:
				lcd_clear_screen();
				lcd_print_hex(30, 40,200,24,24, &p_tag->a_id[8], 8);
				lcd_print_str(0, 1, str_exit_r, 6);
				while ((key = keypad_scan()) == KEYPAD_FLAG_NO);
				if (key == KEYPAD_FLAG_READ)
					state = s_exit;
				break;
			case s_exit:
			default :
				state = s_exit;
				break;
		}
	}
	
	return TRUE;
}




